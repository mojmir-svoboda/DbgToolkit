#include "colorizer_row.h"
#include "constants.h"
#include <serialize/serialize.h>
#include <QPainter>
#include <utils/utils_qstandarditem.h>
#include <utils/utils_color.h>
#include <3rd/qtsln/qtcolorpicker/qtcolorpicker.h>

#include <widgets/logs/logwidget.h> // @TODO: fuck off

ColorizerRow::ColorizerRow (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_ColorizerRow)
	, m_data()
	, m_model(nullptr)
	, m_src_model(nullptr)
{
	initUI();
	setupModel();
}

ColorizerRow::~ColorizerRow ()
{
	destroyModel();
	doneUI();
}

void ColorizerRow::initUI ()
{
	m_ui->setupUi(this);
}

void ColorizerRow::doneUI ()
{
}

void ColorizerRow::actionColor (QModelIndex const & sourceIndex, ColorizedRow const & ct, QColor const & fg, QColor const & bg) const
{
	bool const is_match = ct.m_row == sourceIndex.row();
	if (is_match)
	{
		//QAbstractItemModel * const src_model = const_cast<QAbstractItemModel *>(sourceIndex.model()); // @TODO oh fuck
		int const col_count = m_src_model->columnCount();

		for (int c = 0; c < col_count; ++c)
		{
			// @TODO: skip set colors on level and tid
		//@TODO: if column != level
		//@TODO: if column != tid

			QModelIndex const idx = m_src_model->index(sourceIndex.row(), c, QModelIndex());
			QVariant data = m_src_model->data(idx, Qt::DisplayRole);

			m_src_model->setData(idx, bg, Qt::BackgroundRole);
			m_src_model->setData(idx, fg, Qt::ForegroundRole);
		}
	}
}
void ColorizerRow::actionUncolor (QModelIndex const & sourceIndex, ColorizedRow const & ct) const
{
	actionColor(sourceIndex, ct, QColor(Qt::black), QColor(Qt::white));
}

bool ColorizerRow::action (QModelIndex const & sourceIndex)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedRow const & ct = m_data[i];
		actionColor(sourceIndex, ct, ct.m_fgcolor, ct.m_bgcolor);
	}
	return false;
}

void ColorizerRow::defaultConfig ()
{
	m_data.clear();
}

void ColorizerRow::loadConfig (QString const & path)
{
// 	QString const fname = path + "/" + g_colorizerTag + "/" + typeName();
// 	if (!::loadConfigTemplate(*this, fname))
// 		defaultConfig();
}

void ColorizerRow::saveConfig (QString const & path)
{
// 	QString const fname = path + "/" + g_colorizerTag + "/" + typeName();
// 	::saveConfigTemplate(*this, fname);
}

void ColorizerRow::setConfigToUI ()
{
	m_model->clear();
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, m_data[i].m_row_str);
		if (child == 0)
		{
			add(m_data[i].m_row, m_data[i].m_fgcolor, m_data[i].m_bgcolor);
		}
	}
}

void ColorizerRow::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
	recompileColorRows();
}

void ColorizerRow::clear ()
{
	onSelectNone();
	//m_data.clear();
}


///////// colorizer
void ColorizerRow::setupModel ()
{
	if (!m_model)
	{
		m_model = new QStandardItemModel;
	}
	m_ui->view->setModel(m_model);
	QStringList l;
	l.append("Enabled");
	l.append("Row");
	l.append("fg");
	l.append("bg");
	m_model->setHorizontalHeaderLabels(l);

	//m_ui->view->model()->setHeaderData(0, Qt::Horizontal, "Enabled");
	//m_ui->view->model()->setHeaderData(1, Qt::Horizontal, "Reg Exp");
	//m_ui->view->model()->setHeaderData(2, Qt::Horizontal, "fg");
	//m_ui->view->model()->setHeaderData(3, Qt::Horizontal, "bg");
	//m_ui->view->setSortingEnabled(true);
	m_ui->view->expandAll();

	m_model->setColumnCount(4);
	m_ui->view->setColumnWidth(0, 192);

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->addButton, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(m_ui->comboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onAdd()));
	connect(m_ui->rmButton, SIGNAL(clicked()), this, SLOT(onRm()));
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAt(QModelIndex)));
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));

	m_ui->fgButton->setStandardColors();
	m_ui->fgButton->setCurrentColor(QColor(Qt::black));
	m_ui->bgButton->setStandardColors();
	m_ui->bgButton->setCurrentColor(QColor(Qt::white));
}

void ColorizerRow::destroyModel ()
{
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

ColorizedRow const * ColorizerRow::findMatch (QString const & item) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data.at(i).m_row_str == item)
		{
			return &m_data.at(i);
		}
	return 0;
}

void ColorizerRow::append (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_row_str == item)
		{
			ColorizedRow & ct = m_data[i];
			ct.m_is_enabled = true;
			return;
		}
	m_data.push_back(ColorizedRow(item, Qt::blue, Qt::white));
}
void ColorizerRow::remove (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_row_str == item)
		{
			ColorizedRow & item = m_data[i];
			item.m_is_enabled = false;
			return;
		}
}
ColorizedRow & ColorizerRow::findOrCreateColorizedRow (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_row_str == item)
		{
			ColorizedRow & ct = m_data[i];
			return ct;
		}
	m_data.push_back(ColorizedRow(item, Qt::blue, Qt::white));
	return m_data.back();
}

//////// slots
void ColorizerRow::onSelectAll ()
{
// 	auto fn = &ColorizerRow::append;
// 	applyFnOnAllChildren(fn, m_model, Qt::Checked);
// 	emitFilterChangedSignal();
}

void ColorizerRow::onSelectNone ()
{
// 	auto fn = &ColorizerRow::remove;
// 	applyFnOnAllChildren(fn, m_model, Qt::Unchecked);
// 	emitFilterChangedSignal();
}

void ColorizerRow::onClickedAt (QModelIndex idx)
{
	QStandardItem * item = m_model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & str = m_model->data(idx, Qt::DisplayRole).toString();
	bool const checked = (item->checkState() == Qt::Checked);
	ColorizedRow & ct = findOrCreateColorizedRow(item->text());
	if (checked)
	{
		append(str);
		recompileColorRow(ct);
		updateColor(ct);
	}
	else
	{
		recompileColorRow(ct);
		updateColor(ct);
		remove(str);
	}

	emitFilterChangedSignal();

/*
		if (!idx.isValid()) return;
		QStandardItemModel * model = static_cast<QStandardItemModel *>(m_config_ui.ui()->viewColorRow->model());
		QStandardItem * item = model->itemFromIndex(idx);
		Q_ASSERT(item);

		QString const & val = model->data(idx, Qt::DisplayRole).toString();
		bool const checked = (item->checkState() == Qt::Checked);

		// @TODO: if state really changed
		for (int i = 0, ie = m_filter_state.m_colorized_texts.size(); i < ie; ++i)
		{
			ColorizedRow & ct = m_filter_state.m_colorized_texts[i];
			if (ct.m_row_str == val)
			{
				if (checked)
				{
					m_filter_state.setColorRowChecked(val, checked);
					recompileColorRow(ct);
					updateColor(ct);
				}
				else
				{
					uncolor(ct);
					m_filter_state.setColorRowChecked(val, checked);
					recompileColorRow(ct);
				}
				break;
			}
		}
*/
}


void ColorizerRow::recompile ()
{ }


/*void ColorizerRow::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}*/

void ColorizerRow::updateColor (ColorizedRow const & ct)
{
	int const row = ct.m_row;
	QModelIndex const sourceIndex = m_src_model->index(row, 0, QModelIndex());
	actionColor(sourceIndex, ct, ct.m_fgcolor, ct.m_bgcolor);
}

void ColorizerRow::uncolor (ColorizedRow const & ct)
{
	int const row = ct.m_row;
	QModelIndex const sourceIndex = m_src_model->index(row, 0, QModelIndex());
	actionUncolor(sourceIndex, ct);
}

void ColorizerRow::onActivate (int)
{
}
void ColorizerRow::onFgChanged () { onColorButtonChanged(Qt::ForegroundRole); }
void ColorizerRow::onBgChanged () { onColorButtonChanged(Qt::BackgroundRole); }
void ColorizerRow::onColorButtonChanged (int role)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedRow & ct = m_data[i];
		QStandardItem * const root = m_model->invisibleRootItem();
		QString const qrow = ct.m_row_str;
		QStandardItem * const child = findChildByText(root, qrow);

		if (!child)
			continue;

		QModelIndex const fgidx = m_model->index(child->row(), 1);
		if (fgidx.isValid())
		{
			if (QtColorPicker * w = static_cast<QtColorPicker *>(m_ui->view->indexWidget(fgidx)))
				ct.m_fgcolor = w->currentColor();
		}
		QModelIndex const bgidx = m_model->index(child->row(), 2);
		if (bgidx.isValid())
		{
			if (QtColorPicker * w = static_cast<QtColorPicker *>(m_ui->view->indexWidget(bgidx)))
				ct.m_bgcolor = w->currentColor();
		}

		//TODO: this updates all of them, fixit:413
		//
		updateColor(ct);
	}
}

void ColorizerRow::recompileColorRow (ColorizedRow & ct)
{
	ct.m_is_enabled = true;
}

void ColorizerRow::recompileColorRows ()
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedRow & ct = m_data[i];
		recompileColorRow(ct);
		//updateColor(ct);
	}
}
//void ColorizerRow::onDoubleClickedAtColorRowList (QModelIndex idx) { }

ColorizedRow & ColorizerRow::add (int row, QColor const & fg, QColor const & bg)
{
	QString const row_str = QString::number(row);
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, row_str);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(row_str, true);
		root->appendRow(row_items);
		child = findChildByText(root, row_str);

		QStandardItem * fgitem = new QStandardItem("fg");
		QStandardItem * bgitem = new QStandardItem("bg");
		QStandardItem * stitem = new QStandardItem("status");
		stitem->setCheckable(false);
		m_model->setItem(child->row(), 1, fgitem);
		m_model->setItem(child->row(), 2, bgitem);
		m_model->setItem(child->row(), 3, stitem);
		append(row_str);

		ColorizedRow & ct = findOrCreateColorizedRow(row_str);
		ct.m_fgcolor = fg;
		ct.m_bgcolor = bg;
		{
			QtColorPicker * w = mkColorPicker(m_ui->view, "fg", ct.m_fgcolor);
			connect(w, SIGNAL(colorChanged(const QColor &)), this, SLOT(onFgChanged()));
			QModelIndex const idx = m_model->indexFromItem(fgitem);
			m_ui->view->setIndexWidget(idx, w);
		}
		{
			QtColorPicker * w = mkColorPicker(m_ui->view, "bg", ct.m_bgcolor);
			connect(w, SIGNAL(colorChanged(const QColor &)), this, SLOT(onBgChanged()));
			QModelIndex const idx = m_model->indexFromItem(bgitem);
			m_ui->view->setIndexWidget(idx, w);
		}
		return ct;
	}
	else
	{
		ColorizedRow & ct = findOrCreateColorizedRow(row_str);
		return ct;
	}
}

void ColorizerRow::onAdd ()
{
	QString const qItem = m_ui->comboBox->currentText();
	if (!qItem.length())
		return;
	QColor const qFg = m_ui->fgButton->currentColor();
	QColor const qBg = m_ui->fgButton->currentColor();
	colorize(qItem.toInt(), qFg, qBg);
}

void ColorizerRow::onRm ()
{
	QModelIndex const idx = m_ui->view->currentIndex();
	QStandardItem * item = m_model->itemFromIndex(idx);
	if (!item)
		return;

	QString const & val = m_model->data(idx, Qt::DisplayRole).toString();
	uncolorize(val);
}

void ColorizerRow::colorize (int row, QColor const & fg, QColor const & bg)
{
	ColorizedRow & ct = add(row, fg, bg);

	recompileColorRow(ct);
	updateColor(ct);
}

void ColorizerRow::uncolorize (QString const & row)
{
	QStandardItem * const root = m_model->invisibleRootItem();
	QStandardItem * const child = findChildByText(root, row);
	if (child)
	{
		//QModelIndex const idx = m_model->index(child->row(), 1);
		m_model->removeRow(child->row());
	}

	ColorizedRow & ct = findOrCreateColorizedRow(row);
	recompileColorRow(ct);
	uncolor(ct);
	remove(row);
}

bool ColorizerRow::isRowColored (int row) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedRow const & cr = m_data.at(i);
		if (cr.m_row == row && cr.m_is_enabled)
			return true;
	}
	return false;
}
