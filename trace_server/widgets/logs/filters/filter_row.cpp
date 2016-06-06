#include "filter_row.h"
#include "constants.h"
#include <serialize/serialize.h>
#include <QPainter>
#include <utils/utils_qstandarditem.h>
#include <utils/utils_widgets.h>

namespace logs {

enum E_ColOrder
{
	e_ColOrder_Data = 0,
	e_ColOrder_op,
	e_ColOrder_max_enum_value
};

void FilterRow::setupModelHeader ()
{
	if (m_model->rowCount() == 1)
	{
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_Data, Qt::Horizontal, "Row");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_op, Qt::Horizontal, "Op");
		m_model->setColumnCount(e_ColOrder_max_enum_value);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_Data, 128);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_op, 32);
	}
}


FilterRow::FilterRow (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterRow)
	, m_data()
	, m_model(0)
	, m_delegate(0)
{
	initUI();
	setupModel();
}

FilterRow::~FilterRow ()
{
	destroyModel();
	doneUI();
}

void FilterRow::initUI ()
{
	m_ui->setupUi(this);
}

void FilterRow::doneUI () { }

bool FilterRow::accept (QModelIndex const & sourceIndex)
{
	int const row = sourceIndex.row();
	bool acc = true;
	for (FilteredRow & cfg : m_data)
	{
		acc &= cfg.accept(row);
	}
	return acc;
}

void FilterRow::defaultConfig ()
{
	m_data.clear();
}

void FilterRow::loadConfig (QString const & path)
{
// 	QString const fname = path + "/" + g_filterTag + "/" + typeName();
// 	if (!::loadConfigTemplate(*this, fname))
// 		defaultConfig();
}

void FilterRow::saveConfig (QString const & path)
{
// 	QString const fname = path + "/" + g_filterTag + "/" + typeName();
// 	::saveConfigTemplate(*this, fname);
}

void FilterRow::setConfigToUI ()
{
// 	m_model->clear();
// 	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
// 	{
// 		QStandardItem * root = m_model->invisibleRootItem();
// 		QStandardItem * child = findChildByText(root, m_data[i].m_row_str);
// 		if (child == 0)
// 		{
// 			QList<QStandardItem *> row_items = addRow(m_data[i].m_row_str, true);
// 			row_items[0]->setCheckState(m_data[i].m_is_enabled ? Qt::Checked : Qt::Unchecked);
// 			root->appendRow(row_items);
// 		}
// 	}
}

void FilterRow::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
	//m_ui->view->sortByColumn(0, Qt::AscendingOrder);
}

void FilterRow::clear ()
{
	m_data.clear();
	m_model->clear();
}


///////// row filters
void FilterRow::setupModel ()
{
	if (!m_model)
	{
		m_model = new QStandardItemModel;
	}
	m_ui->view->setModel(m_model);
	m_ui->view->setSortingEnabled(true);
	m_ui->view->expandAll();

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));
	connect(m_model, SIGNAL(dataChanged(QModelIndex const &, QModelIndex const &)), this, SLOT(onDataChanged(QModelIndex const &, QModelIndex const &)));
	connect(m_ui->addButton, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(m_ui->rmButton, SIGNAL(clicked()), this, SLOT(onRm()));
}

void FilterRow::destroyModel ()
{
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

void FilterRow::removeFromConfig (QString const & s)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredRow & cfg = m_data[i];
		if (cfg.m_row_str == s)
		{
			m_data.erase(m_data.begin() + i);
			return;
		}
	}
}

void FilterRow::onRm ()
{
	QModelIndex const idx = m_ui->view->currentIndex();
	QStandardItem * item = m_model->itemFromIndex(idx);
	if (!item)
		return;
	QString const & val = m_model->data(idx, Qt::DisplayRole).toString();
	m_model->removeRow(idx.row());
	removeFromConfig(val);

	emitFilterChangedSignal();
}

void FilterRow::onSelectAll ()
{
}
void FilterRow::onSelectNone ()
{
}

FilteredRow & FilterRow::findOrCreateFilteredRow (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_row_str == item)
		{
			FilteredRow & ct = m_data[i];
			return ct;
		}

	QString const & op_str = m_ui->opBox->currentText();
	E_CmpMode const op = stringToCmpMod(op_str);
	m_data.push_back(FilteredRow(item.toInt(), true, op));
	return m_data.back();
}

void FilterRow::addFilteredRow (FilteredRow const & cfg)
{
	QStandardItem * child = findChildByText(m_model->invisibleRootItem(), cfg.m_row_str);
	if (child == 0)
	{
		addRowToUI(cfg);

// 		std::sort(m_data.begin(), m_data.end());
// 		m_ui->view->sortByColumn(0, Qt::AscendingOrder);

		if (cfg.isValid())
			emitFilterChangedSignal();
	}
	else
	{
		// @TODO
	}
}

void FilterRow::onAdd ()
{
	QString const qItem = m_ui->lineEdit->text();
	if (!qItem.length())
		return;

	QStandardItem * root = m_model->invisibleRootItem();
	FilteredRow & cfg = findOrCreateFilteredRow(qItem);

	addFilteredRow(cfg);
}

void FilterRow::addRowToUI (FilteredRow const & cfg)
{
	int const rows = m_model->rowCount();
	int const row = rows;

	QStandardItem * const name_item = new QStandardItem(cfg.m_row_str);
	name_item->setCheckable(true);
	name_item->setCheckState(true ? Qt::Checked : Qt::Unchecked);

	QStandardItem * opitem = new QStandardItem;

	m_model->blockSignals(true);
	m_model->setItem(row, E_ColOrder::e_ColOrder_Data, name_item);
	m_model->setItem(row, E_ColOrder::e_ColOrder_op, opitem);
	m_model->blockSignals(false);

	QModelIndex const idx = m_model->indexFromItem(opitem);
	QComboBox * cb = new QComboBox;
	copyValuesToUI(m_ui->opBox, cb);
	m_ui->view->setIndexWidget(idx, cb);
	cb->setProperty("orig_model_idx", idx);
	connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(onOperatorChanged(int)));

	setupModelHeader();
}

void FilterRow::onOperatorChanged (int)
{
	QComboBox * src = qobject_cast<QComboBox *>(sender());
	QModelIndex const idx = src->property("orig_model_idx").toModelIndex();
	onDataChanged(idx, idx);
}

void FilterRow::onDataChanged (QModelIndex const & idx, QModelIndex const & parent)
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
			FilteredRow & cfg = findOrCreateFilteredRow(item->text());
			cfg.m_is_enabled = checked;

			emitFilterChangedSignal();
			break;
		}
		case E_ColOrder::e_ColOrder_op:
		{
			QStandardItem * item = m_model->itemFromIndex(idx);
			QWidget * w = m_ui->view->indexWidget(idx);
			QComboBox * cb = qobject_cast<QComboBox *>(w);
			QString const & mod = cb->currentText();
			E_CmpMode const curr_op = stringToCmpMod(mod);
			m_data[r].m_operator = curr_op;
			
			emitFilterChangedSignal();
			break;
		}
	}
}

void FilterRow::locateItem (QString const & item, bool scrollto, bool expand)
{
	/*QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}*/
}

}