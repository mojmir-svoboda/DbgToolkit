#include "colorizer_regex.h"
//#include <tlv_parser/tlv_encoder.h>
#include "constants.h"
#include <serialize/serialize.h>
#include <QPainter>
#include <utils/utils_qstandarditem.h>
#include <utils/utils_color.h>
#include <3rd/qtsln/qtcolorpicker/qtcolorpicker.h>

#include <widgets/logs/logwidget.h> // @TODO: fuck off

ColorizerRegex::ColorizerRegex (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_ColorizerRegex)
	, m_data()
	, m_model(0)
	, m_src_model(0)
{
	initUI();
	setupModel();
}

ColorizerRegex::~ColorizerRegex ()
{
	destroyModel();
	doneUI();
}

void ColorizerRegex::initUI ()
{
	m_ui->setupUi(this);
}

void ColorizerRegex::doneUI ()
{
}

//@TODO: dedup
void ColorizerRegex::actionColor (QModelIndex const & sourceIndex, ColorizedText const & ct, QColor const & fg, QColor const & bg) const
{
	QAbstractItemModel const * const src_model = sourceIndex.model();
	int const col_count = src_model->columnCount();

	for (int c = 0; c < col_count; ++c)
	{
		// @TODO: skip some cols? like tid, lvl, ctx ...
		QModelIndex const idx = src_model->index(sourceIndex.row(), c, QModelIndex());
		QVariant data = src_model->data(idx, Qt::DisplayRole);

		if (bool const is_match = ct.accept(data.toString()))
		{
			if (ct.m_is_enabled && is_match && idx.isValid())
			{
				m_src_model->setData(idx, bg, Qt::BackgroundRole);
				m_src_model->setData(idx, fg, Qt::ForegroundRole);
			}
		}
	}
}
void ColorizerRegex::actionUncolor (QModelIndex const & sourceIndex, ColorizedText const & ct) const
{
	actionColor(sourceIndex, ct, QColor(Qt::black), QColor(Qt::white));
}

bool ColorizerRegex::action (QModelIndex const & sourceIndex)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedText const & ct = m_data[i];
		actionColor(sourceIndex, ct, ct.m_fgcolor, ct.m_bgcolor);
	}
	return false;
}
// 
// bool ColorizerRegex::accept (QModelIndex const & sourceIndex) const
// {
// 	return true;
// }

void ColorizerRegex::defaultConfig ()
{
	m_data.clear();
	m_data.push_back(ColorizedText(".*[Ww]arning.*", QColor(Qt::black), QColor(Qt::yellow)));
	m_data.push_back(ColorizedText(".*[Ee]rror.*", QColor(Qt::black), QColor(Qt::red)));
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedText & ct = m_data[i];
		ct.m_is_enabled = 1;
	}
}

void ColorizerRegex::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_colorizerTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void ColorizerRegex::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_colorizerTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void ColorizerRegex::setConfigToUI ()
{
	m_model->clear();
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, m_data[i].m_regex_str);
		if (child == 0)
		{
			add(m_data[i].m_regex_str, m_data[i].m_fgcolor, m_data[i].m_bgcolor);
		}
	}
}

void ColorizerRegex::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
	recompileColorRegexps();
}


void ColorizerRegex::clear ()
{
	onSelectNone();
	//m_data.clear();
}


///////// colorizer
void ColorizerRegex::setupModel ()
{
	if (!m_model)
	{
		m_model = new QStandardItemModel;
	}
	m_ui->view->setModel(m_model);
	QStringList l;
	l.append("Enabled");
	l.append("Reg Exp");
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
	m_ui->fgButton->setCurrentColor(QColor(Qt::blue));
	m_ui->bgButton->setStandardColors();
	m_ui->bgButton->setCurrentColor(QColor(Qt::white));
}

void ColorizerRegex::destroyModel ()
{
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

ColorizedText const * ColorizerRegex::findMatch (QString const & item) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data.at(i).m_regex_str == item)
		{
			return &m_data.at(i);
		}
	return 0;
}

void ColorizerRegex::append (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_regex_str == item)
		{
			ColorizedText & ct = m_data[i];
			ct.m_is_enabled = true;
			return;
		}
	m_data.push_back(ColorizedText(item, Qt::blue, Qt::white));
}
void ColorizerRegex::remove (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_regex_str == item)
		{
			ColorizedText & item = m_data[i];
			item.m_is_enabled = false;
			return;
		}
}
ColorizedText & ColorizerRegex::findOrCreateColorizedText (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_regex_str == item)
		{
			ColorizedText & ct = m_data[i];
			return ct;
		}
	m_data.push_back(ColorizedText(item, Qt::blue, Qt::white));
	return m_data.back();
}

//////// slots
void ColorizerRegex::onSelectAll ()
{
	auto fn = &ColorizerRegex::append;
	//applyFnOnAllChildren(fn, this, m_model, Qt::Checked);
	emitFilterChangedSignal();
}

void ColorizerRegex::onSelectNone ()
{
	auto fn = &ColorizerRegex::remove;
	//applyFnOnAllChildren(fn, this, m_model, Qt::Unchecked);
	emitFilterChangedSignal();
}

void ColorizerRegex::onClickedAt (QModelIndex idx)
{
	QStandardItem * item = m_model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & str = m_model->data(idx, Qt::DisplayRole).toString();
	bool const checked = (item->checkState() == Qt::Checked);
	ColorizedText & ct = findOrCreateColorizedText(item->text());
	if (checked)
	{
		append(str);
		recompileColorRegex(ct);
		updateColor(ct);
	}
	else
	{
		recompileColorRegex(ct);
		updateColor(ct);
		remove(str);
	}

	emitFilterChangedSignal();
}


void ColorizerRegex::recompile ()
{ }


/*void ColorizerRegex::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}*/

void ColorizerRegex::updateColor (ColorizedText const & ct)
{
// 	for (size_t r = 0, re = m_src_model->dcmds().size(); r < re; ++r)
// 	{
// 		DecodedCommand const & dcmd = m_src_model->dcmds()[r];
// 		actionColor(dcmd, ct, ct.m_fgcolor, ct.m_bgcolor);
// 	}
}

void ColorizerRegex::uncolor (ColorizedText const & ct)
{
// 	for (size_t r = 0, re = m_src_model->dcmds().size(); r < re; ++r)
// 	{
// 		DecodedCommand const & dcmd = m_src_model->dcmds()[r];
// 		actionUncolor(dcmd, ct);
// 	}
}

void ColorizerRegex::onActivate (int)
{
}
void ColorizerRegex::onFgChanged () { onColorButtonChanged(Qt::ForegroundRole); }
void ColorizerRegex::onBgChanged () { onColorButtonChanged(Qt::BackgroundRole); }
void ColorizerRegex::onColorButtonChanged (int role)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedText & ct = m_data[i];
		QStandardItem * const root = m_model->invisibleRootItem();
		QString const qregex = ct.m_regex_str;
		QStandardItem * const child = findChildByText(root, qregex);

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

		//TODO: this updates all of them, fixit
		updateColor(ct);
	}
}

void ColorizerRegex::recompileColorRegex (ColorizedText & ct)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QString const qregex = ct.m_regex_str;
	QStandardItem * child = findChildByText(root, qregex);
	QModelIndex const idx = m_model->indexFromItem(child);
	ct.m_is_enabled = false;
	if (!child)
		return;

	QRegularExpression regex(qregex);
	QString reason;
	if (regex.isValid())
	{
		ct.m_regex = regex;

		bool const checked = (child->checkState() == Qt::Checked);
		if (child && checked)
		{
			child->setData(QBrush(Qt::green), Qt::BackgroundRole);
			reason = "ok";
			ct.m_is_enabled = true;
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
	QStandardItem * item = m_model->item(child->row(), 3);
	item->setText(reason);
}

void ColorizerRegex::recompileColorRegexps ()
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedText & ct = m_data[i];
		recompileColorRegex(ct);
		//updateColor(ct);
	}
}
//void ColorizerRegex::onDoubleClickedAtColorRegexList (QModelIndex idx) { }

ColorizedText & ColorizerRegex::add (QString const & regex, QColor const & fg, QColor const & bg)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, regex);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(regex, true);
		root->appendRow(row_items);
		child = findChildByText(root, regex);

		QStandardItem * fgitem = new QStandardItem("fg");
		QStandardItem * bgitem = new QStandardItem("bg");
		QStandardItem * stitem = new QStandardItem("status");
		stitem->setCheckable(false);
		m_model->setItem(child->row(), 1, fgitem);
		m_model->setItem(child->row(), 2, bgitem);
		m_model->setItem(child->row(), 3, stitem);
		append(regex);

		ColorizedText & ct = findOrCreateColorizedText(regex);
		ct.m_fgcolor = fg;
		ct.m_bgcolor = bg;
		ct.m_is_enabled = true;
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
		ColorizedText & ct = findOrCreateColorizedText(regex);
		return ct;
	}
}

void ColorizerRegex::onAdd ()
{
	QString const qItem = m_ui->comboBox->currentText();
	if (!qItem.length())
		return;
	QColor const qFg = m_ui->fgButton->currentColor();
	QColor const qBg = m_ui->bgButton->currentColor();
	ColorizedText & ct = add(qItem, qFg, qBg);

	recompileColorRegex(ct);
	updateColor(ct);
}

void ColorizerRegex::onRm ()
{
	QModelIndex const idx = m_ui->view->currentIndex();
	QStandardItem * item = m_model->itemFromIndex(idx);
	if (!item)
		return;
	QString const & val = m_model->data(idx, Qt::DisplayRole).toString();
	m_model->removeRow(idx.row());

	ColorizedText & ct = findOrCreateColorizedText(val);
	uncolor(ct);
	remove(val);
}



