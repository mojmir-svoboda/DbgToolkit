#include "filtermgr.h"
#include <algorithm>
#include <QGridLayout>
#include <QComboBox>
#include "ui_combolist.h"
// serialization stuff
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <fstream>
//#include <sstream>

FilterMgr::FilterMgr (QWidget * parent)
	: FilterBase(parent)
{
	m_filters.reserve(e_filtertype_max_value);
	m_cache.resize(e_filtertype_max_value);
}

FilterMgr::~FilterMgr ()
{
}


bool FilterMgr::accept (DecodedCommand const & cmd) const
{
	bool accepted = true;
	for (size_t i = 0, ie = m_filters.size(); i < ie; ++i)
	{
		FilterBase * b = m_filters[i];
		if (b->enabled())
			accepted &= b->accept(cmd);
	}
	return accepted;
}

void FilterMgr::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfig(*this, fname))
		fillDefaultConfig(*this);

	recreateFilters();

	for (size_t i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->loadConfig(path);
}

void FilterMgr::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfig(*this, fname);

	for (size_t i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->saveConfig(path);
}

void FilterMgr::applyConfig ()
{

	for (size_t i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->applyConfig();
}

void FilterMgr::addFilter (FilterBase * b)
{
	E_FilterType const t = b->type();
	m_filters.push_back(b);
	m_cache[t] = b;
}
void FilterMgr::rmFilter (FilterBase * b)
{
	E_FilterType const t = b->type();
	m_filters.erase(std::remove(m_filters.begin(), m_filters.end(), b), m_filters.end());
	m_cache[t] = 0;
}

void FilterMgr::mvFilter (int from, int to)
{
}

FilterBase * filterFactory (E_FilterType t, QWidget * parent)
{
	switch (t)
	{
		case e_Filter_Mgr: return new FilterMgr (parent);
		//case e_Filter_Script: return new FilterScript (parent);
		case e_Filter_String: return new FilterString (parent);
		case e_Filter_Regex: return new FilterRegex (parent);
		case e_Filter_Ctx: return new FilterCtx (parent);
		case e_Filter_Lvl: return new FilterLvl (parent);
		case e_Filter_Tid: return new FilterTid (parent);
		case e_Filter_FileLine: return new FilterFileLine (parent);
		//case e_Filter_User0: return new Filter (parent);
		//case e_Filter_User1: return new Filter (parent);
		//case e_Filter_User2: return new Filter (parent);

		default: return 0;
	}
}

void FilterMgr::recreateFilters ()
{
	m_filters.clear();
	m_filters.resize(m_filter_order.size());
	m_cache.clear();
	m_cache.resize(e_filtertype_max_value);
	for (size_t i = 0, ie = m_filter_order.size(); i < ie; ++i)
	{
		E_FilterType const t = filterName2Type(m_filter_order[i]);
		FilterBase * b = filterFactory(t, this);
		m_filters[i]= b;
		m_cache[t] = b;
	}
}

namespace {
	void fillComboBoxWithFilters (QComboBox * cbx)
	{
		for (size_t i = 0; i < e_filtertype_max_value; ++i)
			cbx->addItem(g_filterNames[i]);
	}

	 
	struct ComboBoxDelegate : public QStyledItemDelegate
	{
		ComboBoxDelegate (QObject *parent = 0);

		QWidget * createEditor (QWidget * parent, QStyleOptionViewItem const & option, QModelIndex const & index) const;
		void setEditorData (QWidget * editor, QModelIndex const & index) const;
		void setModelData (QWidget * editor, QAbstractItemModel * model, QModelIndex const & index) const;
		void updateEditorGeometry (QWidget * editor, QStyleOptionViewItem const & option, QModelIndex const & index) const;
		//Q_OBJECT
	};

	ComboBoxDelegate::ComboBoxDelegate (QObject * parent)
		: QStyledItemDelegate(parent)
	{
	}

	QWidget * ComboBoxDelegate::createEditor (QWidget *parent, QStyleOptionViewItem const & /* option */, QModelIndex const & /* index */) const
	{
		QComboBox * const editor = new QComboBox(parent);
		fillComboBoxWithFilters(editor);
		return editor;
	}

	void ComboBoxDelegate::setEditorData (QWidget * editor, QModelIndex const & index) const
	{
		QString const value = index.model()->data(index, Qt::EditRole).toString();
		QComboBox * const cbx = static_cast<QComboBox *>(editor);
		cbx->setCurrentIndex(cbx->findText(value));
	}

	void ComboBoxDelegate::setModelData(QWidget * editor, QAbstractItemModel *model, QModelIndex const & index) const
	{
		QComboBox * cbx = static_cast<QComboBox *>(editor);
		QString value = cbx->currentText();
		model->setData(index, value, Qt::EditRole);
	}    

	void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const & option, QModelIndex const & /* index */) const
	{
		editor->setGeometry(option.rect);
	}
}

void FilterMgr::initUI ()
{
	setObjectName(QStringLiteral("FilterWidget"));
	//m_widget->setObjectName(QStringLiteral("FilterWidget"));

	//FilterWidget->resize(380, 305);
	QGridLayout * gridLayout = new QGridLayout(this);
	gridLayout->setSpacing(0);
	gridLayout->setContentsMargins(0, 0, 0, 0);
	gridLayout->setObjectName(QStringLiteral("gridLayout"));

	m_tabFilters = new QTabWidget(this);
	m_tabFilters->setObjectName(QStringLiteral("tabFilters"));
	m_tabFilters->setEnabled(true);
	m_tabFilters->setMinimumSize(QSize(128, 0));
	QFont font;
	font.setFamily(QStringLiteral("Verdana"));
	font.setPointSize(7);
	m_tabFilters->setFont(font);
	m_tabFilters->setLayoutDirection(Qt::LeftToRight);
	m_tabFilters->setTabPosition(QTabWidget::North);
	//m_tabFilters->setTabShape(QTabWidget::Triangular);
	m_tabFilters->setUsesScrollButtons(true);

	gridLayout->addWidget(m_tabFilters, 0, 0, 1, 1);
	//m_widget = m_widget;

	m_tabCtxMenu = new ComboList(this);
	MyListModel * model = new MyListModel(this);
	m_tabCtxMenu->ui->filterView->setModel(model);
	m_tabCtxMenu->ui->filterView->setDropIndicatorShown(true);
	m_tabCtxMenu->ui->filterView->setMovement(QListView::Snap);
	m_tabCtxMenu->ui->filterView->setDragDropMode(QAbstractItemView::InternalMove);
	m_delegate = new ComboBoxDelegate(this);
	m_tabCtxMenu->ui->filterView->setItemDelegate(m_delegate);

	connect(m_tabCtxMenu->ui->addButton, SIGNAL(clicked()), this, SLOT(onCtxAddButton()));
	connect(m_tabCtxMenu->ui->rmButton, SIGNAL(clicked()), this, SLOT(onCtxRmButton()));

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
}

void FilterMgr::doneUI ()
{
	disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
	disconnect(m_tabCtxMenu->ui->addButton, SIGNAL(clicked()), this, SLOT(onCtxAddButton()));
	disconnect(m_tabCtxMenu->ui->rmButton, SIGNAL(clicked()), this, SLOT(onCtxRmButton()));
}

void FilterMgr::clearUI ()
{
	if (m_tabCtxModel->hasChildren()) {
		m_tabCtxModel->removeRows(0, m_tabCtxModel->rowCount());
	}
}


void FilterMgr::setConfigToUI ()
{
	clearUI();
	for (size_t i = 0, ie = m_filters.size(); i < ie; ++i)
	{
		QComboBox * cbx = new QComboBox(m_tabCtxMenu);
		fillComboBoxWithFilters(cbx);
		cbx->setCurrentIndex(m_filters[i]->type());
		QStandardItem * const qitem = new QStandardItem();
		//name_item->setCheckable(true);
		//name_item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
		m_tabCtxModel->appendRow(qitem);
	}
}

void FilterMgr::onCtxAddButton ()
{
	QComboBox * cbx = new QComboBox(m_tabCtxMenu);
	fillComboBoxWithFilters(cbx);
	
}

void FilterMgr::onCtxRmButton ()
{
}

void FilterMgr::setUIToConfig ()
{
	//std::vector<FilterBase *> m_origs;
}

void FilterMgr::onShowContextMenu (QPoint const & pt)
{
	setConfigToUI();

	bool const visible = m_tabCtxMenu->isVisible();
	m_tabCtxMenu->setVisible(!visible);

	if (m_tabCtxMenu->isVisible())
	{
		//QPoint globalPos = mapToGlobal(pos);
		m_tabCtxMenu->move(pt);
	}

	//connect(ui->logViewComboBox, SIGNAL(activated(int)), this, SLOT(onLogViewActivate(int)));
}

void FilterMgr::onHideContextMenu ()
{
	m_tabCtxMenu->setVisible(false);
}

void FilterMgr::clear ()
{
}


///////// serialize
bool loadConfig (FilterMgr & w, QString const & fname)
{
	std::ifstream ifs(fname.toLatin1());
	if (!ifs) return false;
	try {
		boost::archive::xml_iarchive ia(ifs);
		ia >> BOOST_SERIALIZATION_NVP(w.m_filter_order);
		ifs.close();
		return true;
	}
	catch (...)
	{
		return false;
	}
}
bool saveConfig (FilterMgr const & w, QString const & fname)
{
	std::ofstream ofs(fname.toLatin1());
	if (!ofs) return false;
	boost::archive::xml_oarchive oa(ofs);
	oa << BOOST_SERIALIZATION_NVP(w.m_filter_order);
	ofs.close();
	return true;
}
void fillDefaultConfig (FilterMgr & w)
{
	w.m_filter_order.push_back(g_filterNames[e_Filter_String]);
	w.m_filter_order.push_back(g_filterNames[e_Filter_Ctx]);
	w.m_filter_order.push_back(g_filterNames[e_Filter_Lvl]);
	w.m_filter_order.push_back(g_filterNames[e_Filter_FileLine]);
}

