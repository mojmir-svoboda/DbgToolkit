#include "colorizer_regex.h"
//#include <tlv_parser/tlv_encoder.h>
#include "constants.h"
#include "serialize.h"
#include <QPainter>
#include "utils_qstandarditem.h"
#include <boost/function.hpp>

/*bool ColorizerRegex::isMatchedColorizedText (QString str, QColor & fgcolor, QColor & bgcolor) const
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

bool ColorizerRegex::accept (DecodedCommand const & cmd) const
{
	QString msg;
	if (!cmd.getString(tlv::tag_msg, msg))
		return true;

	bool enabled = true;
	bool const present = isPresent(msg, enabled);
	return enabled;
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
	connect(m_ui->comboBox, SIGNAL(activated(int)), this, SLOT(onColorRegexActivate(int)));
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

void ColorizerRegex::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}


//////// delegate
ColorizedRegexDelegate::~ColorizedRegexDelegate ()
{
	qDebug("%s", __FUNCTION__);
}
void ColorizedRegexDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
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






