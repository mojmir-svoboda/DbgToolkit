#include "colorizer_regex.h"
//#include <tlv_parser/tlv_encoder.h>
#include "constants.h"
#include "serialize.h"
#include <QPainter>
#include "utils_qstandarditem.h"
#include <boost/function.hpp>
#include <qtsln/qtcolorpicker/qtcolorpicker.h>

#include <logs/logwidget.h> // @TODO: fuck off

/*bool ColorizerRegex::isMatchedColorizerText (QString str, QColor & fgcolor, QColor & bgcolor) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedText const & ct = m_data.at(i);
		if (ct.exactMatch(str))
		{
			fgcolor = ct.m_qcolor;
			bgcolor = ct.m_bgcolor;
			//role = ct.m_role;
			return ct.m_is_enabled;
		}
	}
	return false;
}*/

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

bool ColorizerRegex::action (DecodedCommand const & cmd)
{
	QString msg;
	if (!cmd.getString(tlv::tag_msg, msg))
		return true;

	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedText const & ct = m_data[i];

		bool const is_match = ct.accept(msg);

		int const col = m_src_model->logWidget().findColumn4TagCst(tlv::tag_msg);
		QModelIndex const idx = m_src_model->index(cmd.m_src_row, i, QModelIndex());
		if (is_match && idx.isValid())
		{
			m_src_model->setData(idx, ct.m_fgcolor, Qt::ForegroundRole);
			m_src_model->setData(idx, ct.m_bgcolor, Qt::BackgroundRole);
			return true;
		}
	}

	return false;
}

bool ColorizerRegex::accept (DecodedCommand const & cmd) const
{
	return true;
}

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
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, m_data[i].m_regex_str);
		if (child == 0)
		{
			//FilteredContext & fc = m_data[i];
			QList<QStandardItem *> row_items = addRow(m_data[i].m_regex_str, true);
			row_items[0]->setCheckState(m_data[i].m_is_enabled ? Qt::Checked : Qt::Unchecked);
			root->appendRow(row_items);
		}
	}
}

void ColorizerRegex::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
}


void ColorizerRegex::clear ()
{
	onSelectNone();
	//m_data.clear();
	// @TODO m_ctx_model.clear();
}


///////// colorizer 
void ColorizerRegex::setupModel ()
{
	if (!m_model)
	{
		qDebug("new tree view model");
		m_model = new QStandardItemModel;
	}
	m_ui->view->setModel(m_model);
	//m_ui->view->setSortingEnabled(true);
	m_ui->view->expandAll();

	m_model->setColumnCount(4);
	m_ui->view->setColumnWidth(0, 192);
	ColorizerRegexDelegate * d = new ColorizerRegexDelegate(this);
	m_ui->view->setItemDelegate(d);

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->addButton, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(m_ui->rmButton, SIGNAL(clicked()), this, SLOT(onRm()));
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAt(QModelIndex)));
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));
	m_ui->view->header()->hide();
	//connect(m_ui->comboBox, SIGNAL(activated(int)), this, SLOT(onColorRegexActivate(int)));
}

void ColorizerRegex::destroyModel ()
{
	//if (m_ui->view->itemDelegate())
	//	m_ui->view->setItemDelegate(0);
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

bool ColorizerRegex::isPresent (QString const & item, bool & enabled) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data.at(i).m_regex_str == item)
		{
			ColorizedText const & ct = m_data.at(i);
			enabled = ct.m_is_enabled;
			return true;
		}
	return false;
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
	boost::function<void (ColorizerRegex*, QString)> f = &ColorizerRegex::append;
	applyFnOnAllChildren(f, this, m_model, Qt::Checked);
	emitFilterChangedSignal();
}

void ColorizerRegex::onSelectNone ()
{
	boost::function<void (ColorizerRegex*, QString)> f = &ColorizerRegex::remove;
	applyFnOnAllChildren(f, this, m_model, Qt::Unchecked);
	emitFilterChangedSignal();
}

void ColorizerRegex::onClickedAt (QModelIndex idx)
{
	QStandardItem * item = m_model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & str = m_model->data(idx, Qt::DisplayRole).toString();
	bool const checked = (item->checkState() == Qt::Checked);
	if (checked)
		append(str);
	else
		remove(str);

	emitFilterChangedSignal();

/*
		if (!idx.isValid()) return;
		QStandardItemModel * model = static_cast<QStandardItemModel *>(m_config_ui.ui()->viewColorRegex->model());
		QStandardItem * item = model->itemFromIndex(idx);
		Q_ASSERT(item);

		QString const & val = model->data(idx, Qt::DisplayRole).toString();
		bool const checked = (item->checkState() == Qt::Checked);

		// @TODO: if state really changed
		for (int i = 0, ie = m_filter_state.m_colorized_texts.size(); i < ie; ++i)
		{
			ColorizedText & ct = m_filter_state.m_colorized_texts[i];
			if (ct.m_regex_str == val)
			{
				if (checked)
				{
					m_filter_state.setColorRegexChecked(val, checked);
					recompileColorRegex(ct);
					updateColorRegex(ct);
				}
				else
				{
					uncolorRegex(ct);
					m_filter_state.setColorRegexChecked(val, checked);
					recompileColorRegex(ct);
				}
				break;
			}
		}
*/
}


void ColorizerRegex::recompile ()
{ }

void ColorizerRegex::appendToWidgets (ColorizedText const & ct)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, ct.m_regex_str);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(ct.m_regex_str, true);
		row_items[0]->setCheckState(ct.m_is_enabled ? Qt::Checked : Qt::Unchecked);
		root->appendRow(row_items);
	}
}

/*void ColorizerRegex::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}*/

	void ColorizerRegex::actionColorRegex (DecodedCommand const & cmd, ColorizedText const & ct) const
	{
		for (size_t i = 0, ie = cmd.m_tvs.size(); i < ie; ++i)
		{
			QString const & val = cmd.m_tvs[i].m_val;

			bool const is_match = ct.accept(val);
			//@TODO: cache QMI in DecodedCommand
			QModelIndex const idx = m_src_model->index(cmd.m_src_row, i, QModelIndex());
			if (is_match && idx.isValid())
			{
				m_src_model->setData(idx, ct.m_bgcolor, Qt::BackgroundRole);
				m_src_model->setData(idx, ct.m_fgcolor, Qt::ForegroundRole);
			}

			//@TODO: if column != level
			//@TODO: if column != tid
		}
	}
	void ColorizerRegex::actionUncolorRegex (DecodedCommand const & cmd, ColorizedText const & ct) const
	{
		for (size_t i = 0, ie = cmd.m_tvs.size(); i < ie; ++i)
		{
			QString const & val = cmd.m_tvs[i].m_val;

			bool const is_match = ct.accept(val);
			//@TODO: cache QMI in DecodedCommand
			QModelIndex const idx = m_src_model->index(cmd.m_src_row, i, QModelIndex());
			if (is_match && idx.isValid())
			{
				m_src_model->setData(idx, QColor(Qt::white), Qt::BackgroundRole);
				m_src_model->setData(idx, QColor(Qt::black), Qt::ForegroundRole);
			}
			//@TODO: if column != level
			//@TODO: if column != tid
		}
	}


	void ColorizerRegex::updateColorRegex (ColorizedText const & ct)
	{
		for (size_t r = 0, re = m_src_model->dcmds().size(); r < re; ++r)
		{
			DecodedCommand const & dcmd = m_src_model->dcmds()[r];
			actionColorRegex(dcmd, ct);
		}
	}


	void ColorizerRegex::uncolorRegex (ColorizedText const & ct)
	{
		for (size_t r = 0, re = m_src_model->dcmds().size(); r < re; ++r)
		{
			DecodedCommand const & dcmd = m_src_model->dcmds()[r];
			actionUncolorRegex(dcmd, ct);
		}
	}

	void ColorizerRegex::onActivate (int)
	{
    }
	void ColorizerRegex::onFgChanged () { onColorRegexChanged(Qt::ForegroundRole); }
	void ColorizerRegex::onBgChanged () { onColorRegexChanged(Qt::BackgroundRole); }
	void ColorizerRegex::onColorRegexChanged (int role)
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
			//TODO
			//updateColorRegex();
		}
	}

	QtColorPicker * mkColorPicker (QWidget * parent, QString const & txt, QColor const & c)
	{
		QtColorPicker * w = new QtColorPicker(parent, txt);
		w->setStandardColors();
		w->setCurrentColor(c);
		return w;
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

		QRegExp regex(qregex);
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

	/*void ColorizerRegex::recompileColorRegexps ()
	{
		for (int i = 0, ie = m_filter_state.m_colorized_texts.size(); i < ie; ++i)
		{
			ColorizedText & ct = m_filter_state.m_colorized_texts[i];
			recompileColorRegex(ct);
			updateColorRegex(ct);
		}
	}
	void ColorizerRegex::onDoubleClickedAtColorRegexList (QModelIndex idx) { }*/

	void ColorizerRegex::onAdd ()
	{
		QString qItem = m_ui->comboBox->currentText();
		if (!qItem.length())
			return;
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, qItem);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addRow(qItem, true);
			root->appendRow(row_items);
			child = findChildByText(root, qItem);

			QStandardItem * fgitem = new QStandardItem("fg");
			QStandardItem * bgitem = new QStandardItem("bg");
			QStandardItem * stitem = new QStandardItem("status");
			stitem->setCheckable(false);
			m_model->setItem(child->row(), 1, fgitem);
			m_model->setItem(child->row(), 2, bgitem);
			m_model->setItem(child->row(), 3, stitem);
			append(qItem);

            ColorizedText & ct = findOrCreateColorizedText(qItem);
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
            recompileColorRegex(ct);
			updateColorRegex(ct);
		}
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
        uncolorRegex(ct);
		remove(val);
	}



//////// delegate
ColorizerRegexDelegate::~ColorizerRegexDelegate ()
{
	qDebug("%s", __FUNCTION__);
}
void ColorizerRegexDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

	/*if (m_app_data && m_app_data->getDictCtx().m_names.size())
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Dict const & dict = m_app_data->getDictCtx();

			option4.text = dict.findNameFor(value.toString());
			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}
	else*/
		QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}






