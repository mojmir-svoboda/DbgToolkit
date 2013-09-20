#include "filter_regex.h"
#include "constants.h"
#include "serialize.h"
#include <QPainter>
#include "utils_qstandarditem.h"
#include <boost/function.hpp>
// serialization stuff
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <fstream>

FilterRegex::FilterRegex (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterRegex)
	, m_data()
	, m_model(0)
	, m_delegate(0)
{
	initUI();
	setupModel();
}

FilterRegex::~FilterRegex ()
{
	destroyModel();
	doneUI();
}

void FilterRegex::initUI ()
{
	m_ui->setupUi(this);
}

void FilterRegex::doneUI ()
{
}

bool FilterRegex::accept (DecodedCommand const & cmd) const
{
	bool inclusive_filters = false;
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredRegex const & fr = m_data.at(i);
		if (!fr.m_is_enabled)
			continue;
		else
		{
			if (fr.m_state)
			{
				inclusive_filters = true;
				break;
			}
		}
	}
	if (m_data.size() > 0)
	{
		QString msg;
		if (!cmd.getString(tlv::tag_msg, msg))
			return true;

		for (int i = 0, ie = m_data.size(); i < ie; ++i)
		{
			FilteredRegex const & fr = m_data.at(i);
			if (fr.exactMatch(msg))
			{
				if (!fr.m_is_enabled)
					continue;
				else
				{
					if (fr.m_state)
						return true;
					else
						return false;
				}
			}
		}

	}
	return true;
}

void FilterRegex::defaultConfig ()
{
	m_data.clear();
}

void FilterRegex::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}
void FilterRegex::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterRegex::applyConfig ()
{
	FilterBase::applyConfig();
	//m_filter_state.m_filtered_regexps = src.m_filtered_regexps;
}

void FilterRegex::clear ()
{
	m_data.clear();
	// @TODO m_regex_model.clear();
}

///////////////////

void FilterRegex::setupModel ()
{
	if (!m_model)
		m_model = new QStandardItemModel;
	m_ui->view->setModel(m_model);
	m_ui->view->setItemDelegate(new RegexDelegate(this));

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_ui->view->header()->hide();
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtRegexList(QModelIndex)));
	connect(m_ui->comboBoxRegex, SIGNAL(activated(int)), this, SLOT(onRegexActivate(int)));
	connect(m_ui->buttonAddRegex, SIGNAL(clicked()), this, SLOT(onRegexAdd()));
	connect(m_ui->buttonRmRegex, SIGNAL(clicked()), this, SLOT(onRegexRm()));
}

void FilterRegex::destroyModel ()
{
	if (m_ui->view->itemDelegate())
		m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
	delete m_delegate;
	m_delegate = 0;
}

bool FilterRegex::isMatchedRegexExcluded (QString str) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredRegex const & fr = m_data.at(i);
		if (fr.exactMatch(str))
		{
			if (!fr.m_is_enabled)
				return false;
			else
			{
				return fr.m_state ? false : true;
			}
		}
	}
	return false;
}
void FilterRegex::setRegexInclusive (QString const & s, bool state)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_data[i];
		if (fr.m_regex_str == s)
		{
			fr.m_state = state;
		}
	}
}
void FilterRegex::setRegexChecked (QString const & s, bool checked)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_data[i];
		if (fr.m_regex_str == s)
		{
			fr.m_is_enabled = checked;
		}
	}
}
void FilterRegex::removeFromRegexFilters (QString const & s)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_data[i];
		if (fr.m_regex_str == s)
		{
			m_data.removeAt(i);
			return;
		}
	}
}
void FilterRegex::appendToRegexFilters (QString const & s, bool enabled, bool state)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_regex_str == s)
			return;
	m_data.push_back(FilteredRegex(s, enabled, state));
}

void FilterRegex::onClickedAtRegexList (QModelIndex idx)
{
	if (!idx.isValid())
		return;
/*	QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetRegex()->model());

	if (idx.column() == 1)
	{
		QString const & mod = model->data(idx, Qt::DisplayRole).toString();
		E_FilterMode const curr = stringToFltMod(mod.toStdString().c_str()[0]);
		size_t const i = (curr + 1) % e_max_fltmod_enum_value;
		E_FilterMode const new_mode = static_cast<E_FilterMode>(i);
		model->setData(idx, QString(fltModToString(new_mode)));

		if (Connection * conn = findCurrentConnection())
		{
			QString const & reg = model->data(model->index(idx.row(), 0, QModelIndex()), Qt::DisplayRole).toString();
			bool const is_inclusive = new_mode == e_Include;
			conn->sessionState().setRegexInclusive(reg, is_inclusive);
			conn->recompileRegexps();
			conn->onInvalidateFilter();
		}
	}
	else
	{
		QString const & mod = model->data(model->index(idx.row(), 1, QModelIndex()), Qt::DisplayRole).toString();
		E_FilterMode const curr = stringToFltMod(mod.toStdString().c_str()[0]);

		QStandardItem * item = model->itemFromIndex(idx);
		Q_ASSERT(item);
		bool const orig_checked = (item->checkState() == Qt::Checked);
		Qt::CheckState const checked = orig_checked ? Qt::Unchecked : Qt::Checked;
		item->setCheckState(checked);
		if (Connection * conn = findCurrentConnection())
		{
			// @TODO: if state really changed
			QString const & val = model->data(idx, Qt::DisplayRole).toString();
			bool const is_inclusive = curr == e_Include;
			conn->sessionState().setRegexInclusive(val, is_inclusive);
			conn->m_session_state.setRegexChecked(val, checked);
			conn->recompileRegexps();
			conn->onInvalidateFilter();
		}
	}*/
}

void FilterRegex::recompile ()
{
/*	for (int i = 0, ie = filterMgr()->getFilterRegex()->m_data.size(); i < ie; ++i)
	{
		FilteredRegex & fr = filterMgr()->getFilterRegex()->m_data[i];
		QStandardItem * root = filterMgr()->getFilterRegex()->m_model->invisibleRootItem();
		QString const qregex = fr.m_regex_str;
		QStandardItem * child = findChildByText(root, qregex);
		fr.m_is_enabled = false;
		if (!child)
			continue;
		QRegExp regex(qregex);
		if (regex.isValid())
		{
			fr.m_regex = regex;
			bool const checked = (child->checkState() == Qt::Checked);
			if (child && checked)
			{
				child->setData(QBrush(Qt::green), Qt::BackgroundRole);
				child->setToolTip(tr("ok"));
				fr.m_is_enabled = true;
			}
			else if (child && !checked)
			{
				child->setData(QBrush(Qt::yellow), Qt::BackgroundRole);
				child->setToolTip(tr("regex not enabled"));
			}
		}
		else
		{
			if (child)
			{
				child->setData(QBrush(Qt::red), Qt::BackgroundRole);
				child->setToolTip(regex.errorString());
			}
		}
	}

	onInvalidateFilter();*/
}

void FilterRegex::onRegexActivate (int idx)
{
	onRegexAdd();
}

void FilterRegex::onRegexAdd ()
{
	QString qItem = m_ui->comboBoxRegex->currentText();
	//onRegexAdd(qItem);
/*
 *
	if (!qItem.length())
		return;
	QStandardItem * root = static_cast<QStandardItemModel *>(getWidgetRegex()->model())->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addTriRow(qItem, Qt::Unchecked, true);
		root->appendRow(row_items);
		conn->appendToRegexFilters(qItem, false, true);
		conn->recompileRegexps();
	}
	*/
}

void FilterRegex::onRegexRm ()
{
	/*QStandardItemModel * model = static_cast<QStandardItemModel *>(getWidgetRegex()->model());
	QModelIndex const idx = getWidgetRegex()->currentIndex();
	QStandardItem * item = model->itemFromIndex(idx);
	if (!item)
		return;
	QString const & val = model->data(idx, Qt::DisplayRole).toString();
	model->removeRow(idx.row());*/
	//Connection * conn = m_server->findCurrentConnection();
	//if (conn)
	{
		//conn->onRegexRm();
		//conn->removeFromRegexFilters(val);
		//conn->recompileRegexps();
	}
}

void FilterRegex::onStringAdd ()
{
	//Connection * conn = m_server->findCurrentConnection();
	//if (!conn) return;

	//QString qItem = ui->qFilterLineEdit->text();
	//conn->onStringAdd(qItem);

	/*
	 *
	 *
	if (!qItem.length())
		return;
	 * QStandardItem * root = static_cast<QStandardItemModel *>(getWidgetString()->model())->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addTriRow(qItem, Qt::Checked, true);
		root->appendRow(row_items);
		conn->appendToStringFilters(qItem, true, true);
		row_items[0]->setCheckState(Qt::Checked);
		conn->recompileStrings();
	}*/
}






//////// delegate
void RegexDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

	if (index.column() == 1)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();

			E_FilterMode const mode = stringToFltMod(qs.at(0).toLatin1());
			QString const verbose = fltmodsStr[mode];
			option4.text = verbose;

			if (QWidget const * widget = option4.widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}

	}
	else
		QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}

