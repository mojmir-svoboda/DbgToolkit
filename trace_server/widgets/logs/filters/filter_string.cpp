#include "filter_string.h"
#include "constants.h"
#include <serialize/serialize.h>
#include <utils/utils_qstandarditem.h>
#include <utils/utils_widgets.h>
#include <QPainter>
#include <QComboBox>

namespace logs {

FilterString::FilterString (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterString)
	, m_data()
	, m_model(0)
{
	initUI();
	setupModel();
}

FilterString::~FilterString ()
{
	destroyModel();
	doneUI();
}

void FilterString::initUI ()
{
	m_ui->setupUi(this);
}

void FilterString::doneUI ()
{
}

enum E_ColOrder
{
	e_ColOrder_Data = 0,
	e_ColOrder_neg,
	e_ColOrder_aA,
	e_ColOrder_ww,
	e_ColOrder_useRegExp,
	e_ColOrder_where,
	e_ColOrder_status,
	e_ColOrder_max_enum_value
};

void FilterString::setupModelHeader ()
{
	if (m_model->rowCount() == 1)
	{
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_Data, Qt::Horizontal, "String/RegExp");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_neg, Qt::Horizontal, "!");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_aA, Qt::Horizontal, "aA");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_ww, Qt::Horizontal, "ww");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_useRegExp, Qt::Horizontal, ".*");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_where, Qt::Horizontal, "where");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_status, Qt::Horizontal, "state");
		m_model->setColumnCount(e_ColOrder_max_enum_value);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_Data, 192);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_neg, 32);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_aA, 32);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_ww, 32);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_useRegExp, 32);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_where, 128);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_status, 64);
	}
}

void FilterString::recompileRegex (FilteredString & cfg)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QString const str = cfg.m_regex_str;
	QStandardItem * child = findChildByText(root, str);
	bool const was_enabled = cfg.m_is_enabled;
	if (cfg.m_is_regex)
	{
		QModelIndex const idx = m_model->indexFromItem(child);
		cfg.m_is_enabled = false;
		if (!child)
			return;

		QRegularExpression regex(str);
		QString reason;
		if (regex.isValid())
		{
			cfg.m_regex = regex;

			bool const checked = (child->checkState() == Qt::Checked);
			if (child && checked)
			{
				child->setData(QBrush(Qt::green), Qt::BackgroundRole);
				reason = "ok";
				cfg.m_is_enabled = was_enabled;
			}
			else if (child && !checked)
			{
				child->setData(QBrush(Qt::yellow), Qt::BackgroundRole);
				reason = "not checked";
			}
		}
		else
		{
			if (child)
			{
				child->setData(QBrush(Qt::red), Qt::BackgroundRole);
				reason = regex.errorString();
			}
		}

		child->setToolTip(reason);
		QStandardItem * item = m_model->item(child->row(), e_ColOrder_status);
		item->setText(reason);
	}
	else
	{
		//child->setData(QBrush(Qt::green), Qt::BackgroundRole);
		//QString const reason = "ok";

		//child->setToolTip(reason);
		//QStandardItem * item = m_model->item(child->row(), e_ColOrder_status);
		//item->setText(reason);
	}
}

bool FilterString::accept (QModelIndex const & sourceIndex)
{
	if (m_data.size() == 0)
		return true; // no strings at all

	QAbstractItemModel const * const src_model = sourceIndex.model();
	int const col_count = src_model->columnCount();

	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString const & fr = m_data.at(i);

		bool const negate = (fr.m_negate_match == static_cast<unsigned>(E_FilterMode::e_Negate));
		bool matched = false;

		for (int c = 0, ce = src_model->columnCount(); c < ce; ++c)
		{
			if (!fr.m_where.m_states[c])
				continue;

			QModelIndex const idx = src_model->index(sourceIndex.row(), c, QModelIndex());
			QVariant data = src_model->data(idx, Qt::DisplayRole);
			if (fr.accept(data.toString()))
			{
				if (!fr.m_is_enabled)
					continue;
				else
				{
					matched = true;
					if (negate)
						return true;
					else
						return false;
				}
			}
		}

		if (fr.m_is_enabled && negate && !matched)
			return false;
	}

	return true;
}

void FilterString::defaultConfig ()
{
	m_data.clear();
}

void FilterString::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterString::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterString::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
}

void FilterString::clear ()
{
	m_data.clear();
}


///////////////////
void FilterString::setupModel ()
{
	if (!m_model)
		m_model = new QStandardItemModel;
	m_ui->view->setModel(m_model);
	connect(m_ui->qFilterLineEdit, SIGNAL(returnPressed()), this, SLOT(onStringAdd()));
	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtStringList(QModelIndex)));
	//connect(m_ui->view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedAtStringList(QModelIndex)));
	//connect(ui->comboBoxString, SIGNAL(activated(int)), this, SLOT(onStringActivate(int)));
	connect(m_ui->buttonAddString, SIGNAL(clicked()), this, SLOT(onStringAdd()));
	connect(m_ui->buttonRmString, SIGNAL(clicked()), this, SLOT(onStringRm()));
	connect(m_model, SIGNAL(dataChanged(QModelIndex const &, QModelIndex const &)), this, SLOT(onDataChanged(QModelIndex const &, QModelIndex const &)));
}

void FilterString::destroyModel ()
{
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

void FilterString::setDefaultSearchConfig (CheckedComboBoxConfig const& cccfg)
{
	m_cccfg = cccfg;
	setValuesToUI(m_ui->whereComboBox, cccfg);
}



void FilterString::removeFromStringFilters (QString const & s)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString & fr = m_data[i];
		if (fr.m_regex_str == s)
		{
			m_data.removeAt(i);
			return;
		}
	}
	emitFilterChangedSignal();
}
// void FilterString::appendToStringFilters (bool on, bool neg, bool is_regex, bool cs, bool ww, QString const & s, CheckedComboBoxConfig & cccfg)
// {
// 	for (int i = 0, ie = m_data.size(); i < ie; ++i)
// 		if (m_data[i].m_regex_str == s)
// 			return;
// 
// 	m_data.push_back(FilteredString(on, neg, is_regex, cs, ww, s, cccfg));
// 
// 	setConfigToUI();
// 	emitFilterChangedSignal();
// }
FilteredString & FilterString::findOrCreateFilteredString (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_regex_str == item)
		{
			FilteredString & ct = m_data[i];
			return ct;
		}
	m_data.push_back(FilteredString(false, false, false, true, false, item, m_cccfg));
	return m_data.back();
}

/*void FilterString::appendToStringFilters (QString const & s, QString const & col)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_string == s)
			return;
	m_data.push_back(FilteredString(s, true, col));
	emitFilterChangedSignal();
}*/

void FilterString::recompile ()
{ }

void FilterString::onStringRm ()
{
	QModelIndex const idx = m_ui->view->currentIndex();
	QStandardItem * item = m_model->itemFromIndex(idx);
	if (!item)
		return;
	QString const & val = m_model->data(idx, Qt::DisplayRole).toString();
	m_model->removeRow(idx.row());
	removeFromStringFilters(val);
	recompile();

	emitFilterChangedSignal();
}

void FilterString::setConfigToUI ()
{
	m_model->clear();
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		addRowToUI(m_data[i]);
		setupModelHeader();
	}
}

void FilterString::setUIValuesToConfig (FilteredString & cfg)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, cfg.m_regex_str);
	if (child == 0)
	{
		cfg.m_is_enabled = true;
		cfg.m_negate_match = m_ui->negButton->isChecked();
		cfg.m_case_sensitive = m_ui->aAButton->isChecked();
		cfg.m_whole_word = m_ui->wwButton->isChecked();
		cfg.m_is_regex = m_ui->regexButton->isChecked();
		::setUIValuesToConfig(m_ui->whereComboBox, cfg.m_where);

	}
	else
	{
		int const row = child->row();
		QVariant qv_enabled = m_model->data(m_model->index(row, e_ColOrder_Data, QModelIndex()), Qt::CheckStateRole);
		cfg.m_is_enabled = qv_enabled.toBool();
	}
}

void FilterString::onStringAdd ()
{
	QString const qItem = m_ui->qFilterLineEdit->text();
	if (!qItem.length())
		return;

	QStandardItem * root = m_model->invisibleRootItem();
	FilteredString & cfg = findOrCreateFilteredString(qItem);
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		setUIValuesToConfig(cfg);
		addRowToUI(cfg);
		recompileRegex(cfg);
		//recompile();
		if (cfg.isValid())
			emitFilterChangedSignal();
	}
}

void FilterString::addRowToUI (FilteredString const & cfg)
{
	int const rows = m_model->rowCount();
	int const row = rows;

	QStandardItem * const name_item = new QStandardItem(cfg.m_regex_str);
	name_item->setCheckable(true);
	name_item->setCheckState(true ? Qt::Checked : Qt::Unchecked);
	QStandardItem * negitem = new QStandardItem;
	negitem->setCheckable(true);
	negitem->setCheckState(cfg.m_negate_match ? Qt::Checked : Qt::Unchecked);
	QStandardItem * aAitem = new QStandardItem;
	aAitem->setCheckable(true);
	aAitem->setCheckState(cfg.m_case_sensitive ? Qt::Checked : Qt::Unchecked);
	QStandardItem * wwitem = new QStandardItem;
	wwitem->setCheckable(true);
	wwitem->setCheckState(cfg.m_whole_word ? Qt::Checked : Qt::Unchecked);
	QStandardItem * regexitem = new QStandardItem;
	regexitem->setCheckable(true);
	regexitem->setCheckState(cfg.m_is_regex ? Qt::Checked : Qt::Unchecked);
	QStandardItem * wavitem = new QStandardItem;
	QStandardItem * whereitem = new QStandardItem;
	QStandardItem * stitem = new QStandardItem;
	stitem->setCheckable(false);

	m_model->setItem(row, E_ColOrder::e_ColOrder_Data, name_item);
	m_model->setItem(row, E_ColOrder::e_ColOrder_neg, negitem);
	m_model->setItem(row, E_ColOrder::e_ColOrder_aA, aAitem);
	m_model->setItem(row, E_ColOrder::e_ColOrder_ww, wwitem);
	m_model->setItem(row, E_ColOrder::e_ColOrder_useRegExp, regexitem);
	m_model->setItem(row, E_ColOrder::e_ColOrder_where, whereitem);
	m_model->setItem(row, E_ColOrder::e_ColOrder_status, stitem);

	QModelIndex const idx = m_model->indexFromItem(whereitem);
	QComboBox * cb = new QComboBox;
	m_ui->view->setIndexWidget(idx, cb);
	setValuesToUI(cb, m_cccfg);

	QModelIndex const wavidx = m_model->indexFromItem(wavitem);
	QComboBox * wavcb = new QComboBox;
	m_ui->view->setIndexWidget(wavidx, wavcb);

	setupModelHeader();
}

void FilterString::onDataChanged (QModelIndex const & idx, QModelIndex const & parent)
{
	QAbstractItemModel const * const m = idx.model();
	int const r = idx.row();
	switch (idx.column())
	{
		case E_ColOrder::e_ColOrder_Data:
		{
			QStandardItem * item = m_model->itemFromIndex(idx);
			Q_ASSERT(item);

			QString const & str = m_model->data(idx, Qt::DisplayRole).toString();
			bool const checked = (item->checkState() == Qt::Checked);
			FilteredString & cfg = findOrCreateFilteredString(item->text());
			cfg.m_is_enabled = checked;
			recompileRegex(cfg);

			if (cfg.isValid())
				emitFilterChangedSignal();
			break;
		}
		case E_ColOrder::e_ColOrder_neg:
		{
			QVariant val = m->data(idx, Qt::CheckStateRole);
			bool const checked = val == Qt::Checked;
			m_data[r].m_negate_match = checked;

			emitFilterChangedSignal();
			break;
		}
		case E_ColOrder::e_ColOrder_aA:
		{
			QVariant val = m->data(idx, Qt::CheckStateRole);
			bool const checked = val == Qt::Checked;
			m_data[r].m_case_sensitive = checked;

			emitFilterChangedSignal();
			break;
		}
		case E_ColOrder::e_ColOrder_ww:
		{
			QVariant val = m->data(idx, Qt::CheckStateRole);
			bool const checked = val == Qt::Checked;
			m_data[r].m_whole_word = checked;

			emitFilterChangedSignal();
			break;
		}
		case E_ColOrder::e_ColOrder_useRegExp:
		{
			QVariant val = m->data(idx, Qt::CheckStateRole);
			bool const checked = val == Qt::Checked;
			m_data[r].m_is_regex = checked;
			FilteredString & cfg = m_data[r];
			recompileRegex(cfg);

			if (cfg.isValid())
				emitFilterChangedSignal();
			break;
		}
		case E_ColOrder::e_ColOrder_where:
			break;
		case E_ColOrder::e_ColOrder_status:
			break;
	}
}


void FilterString::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}

}