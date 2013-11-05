#include "filtermgr.h"
#include <algorithm>
#include <QGridLayout>
#include <QComboBox>
#include <QModelIndexList>
#include "ui_combolist.h"
#include "serialize.h"
// serialization stuff
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <fstream>

FilterMgr::FilterMgr (QWidget * parent)
	: FilterBase(parent)
	, m_tabFilters(0)
	, m_tabCtxMenu(0)
	, m_delegate(0)
	, m_tabCtxModel(0)
{
	m_filters.reserve(e_filtertype_max_value);
	m_filter_order.reserve(e_filtertype_max_value);
	m_cache.resize(e_filtertype_max_value);
	initUI();
}

FilterMgr::~FilterMgr ()
{
	qDebug("%s", __FUNCTION__);
}


bool FilterMgr::accept (DecodedCommand const & cmd) const
{
	bool accepted = true;
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
	{
		FilterBase * b = m_filters[i];
		if (b->enabled())
			accepted &= b->accept(cmd);
	}
	return accepted;
}

void FilterMgr::defaultConfig ()
{
	m_filter_order.clear();
	// @TODO clear others? probably no
	m_filter_order.push_back(g_filterNames[e_Filter_String]);
	m_filter_order.push_back(g_filterNames[e_Filter_Ctx]);
	m_filter_order.push_back(g_filterNames[e_Filter_Lvl]);
	m_filter_order.push_back(g_filterNames[e_Filter_FileLine]);
}

void FilterMgr::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();

	recreateFilters();

	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->loadConfig(path);
}

void FilterMgr::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);

	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->saveConfig(path);
}

void FilterMgr::applyConfig ()
{
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->applyConfig();
}


bool FilterMgr::someFilterEnabled () const
{
	bool some_filter_enabled = false;
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		if (m_filters[i] && m_filters[i]->enabled())
			some_filter_enabled |= 1;
	return some_filter_enabled;
}

bool FilterMgr::enabled () const
{
	return m_enabled && someFilterEnabled();
}

void FilterMgr::addFilter (FilterBase * b)
{
	E_FilterType const t = b->type();
	m_filters.push_back(b);
	m_cache[t] = b;
}
void FilterMgr::rmFilter (FilterBase * & b)
{
	E_FilterType const t = b->type();
	m_filters.erase(std::remove(m_filters.begin(), m_filters.end(), b), m_filters.end());
	delete b;
	b = 0;
	m_cache[t] = 0;
}

void FilterMgr::mvFilter (int from, int to)
{
	m_filters.move(from, to);
	m_filter_order.move(from, to);
}

void FilterMgr::onTabMoved (int from, int to)
{
	mvFilter(from, to);
	//TODO: emit signal to recalc model?
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

void FilterMgr::connectFiltersTo (QWidget * w)
{
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		connect(m_filters[i], SIGNAL(filterChangedSignal()), w, SLOT(onFilterChanged()));
}

void FilterMgr::disconnectFiltersTo (QWidget * w)
{
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		disconnect(m_filters[i], SIGNAL(filterChangedSignal()), w, SLOT(onFilterChanged()));
}


void FilterMgr::recreateFilters ()
{
	m_filters.clear();
	m_filters.reserve(m_filter_order.size());
	//m_cache.clear();
	m_cache.resize(e_filtertype_max_value);

	for (int i = 0, ie = m_tabFilters->tabBar()->count(); i < ie; ++i)
	{
		m_tabFilters->tabBar()->setTabButton(i, QTabBar::LeftSide, 0);
	}

	m_tabFilters->clear();
	for (int i = 0, ie = m_filter_order.size(); i < ie; ++i)
	{
		E_FilterType const t = filterName2Type(m_filter_order[i]);
		FilterBase * fb = 0;
		if (m_cache[t] != 0)
		{
			fb = m_cache[t];
		}
		else
		{
			fb = filterFactory(t, 0);
			connect(fb, SIGNAL(filterEnabledChanged()), this, SLOT(onFilterEnabledChanged()));
			m_cache[t] = fb;
		}
		m_filters.push_back(fb);
		m_tabFilters->insertTab(i, fb, fb->typeName());
		m_tabFilters->tabBar()->setTabButton(i, QTabBar::LeftSide, fb->m_button);
	}

	for (size_t c = 0, ce = m_cache.size(); c < ce; ++c)
	{
		if (m_cache[c] == 0)
			continue;

		bool used = 0;
		for (int i = 0, ie = m_filters.size(); i < ie; ++i)
			if (m_cache[c] && m_cache[c] == m_filters[i])
			{
				used |= 1;
				break;
			}
		if (used)
			continue;

		delete m_cache[c];
		m_cache[c] = 0;
	}
}

namespace {
	void fillComboBoxWithFilters (QComboBox * cbx)
	{
		for (int i = 0; i < e_filtertype_max_value; ++i)
			cbx->addItem(g_filterNames[i]);
	}

	struct ComboBoxDelegate : public QStyledItemDelegate
	{
		QComboBox mutable * m_cbx;
		ComboBoxDelegate (QObject * parent = 0);

		QWidget * createEditor (QWidget * parent, QStyleOptionViewItem const & option, QModelIndex const & index) const;
		void setEditorData (QWidget * editor, QModelIndex const & index) const;
		void setModelData (QWidget * editor, QAbstractItemModel * model, QModelIndex const & index) const;
		void updateEditorGeometry (QWidget * editor, QStyleOptionViewItem const & option, QModelIndex const & index) const;
		//Q_OBJECT
		QSize sizeHint (QStyleOptionViewItem const & option, QModelIndex const & index) const
		{
			return QSize(64, 25);
		}
	};

	ComboBoxDelegate::ComboBoxDelegate (QObject * parent)
		: QStyledItemDelegate(parent)
	{
	}

	QWidget * ComboBoxDelegate::createEditor (QWidget * parent, QStyleOptionViewItem const & /* option */, QModelIndex const & /* index */) const
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

	void ComboBoxDelegate::setModelData (QWidget * editor, QAbstractItemModel *model, QModelIndex const & index) const
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

	resize(380, 305);
	QGridLayout * gridLayout = new QGridLayout(this);
	gridLayout->setSpacing(0);
	gridLayout->setContentsMargins(0, 0, 0, 0);
	gridLayout->setObjectName(QStringLiteral("gridLayout"));

	m_tabFilters = new MovableTabWidget(this);
	m_tabFilters->setObjectName(QStringLiteral("tabFilters"));
	m_tabFilters->setEnabled(true);
	m_tabFilters->setMinimumSize(QSize(128, 0));
	m_tabFilters->setMovable(true);
	QFont font;
	font.setFamily(QStringLiteral("Verdana"));
	font.setPointSize(7);
	m_tabFilters->setFont(font);
	m_tabFilters->setLayoutDirection(Qt::LeftToRight);
	m_tabFilters->setTabPosition(QTabWidget::North);
	m_tabFilters->setUsesScrollButtons(true);
	connect(m_tabFilters, SIGNAL(tabMovedSignal(int, int)), this, SLOT(onTabMoved(int, int)));

	gridLayout->addWidget(m_tabFilters, 0, 0, 1, 1);
	setLayout(gridLayout);
	//m_widget = m_widget;

	m_tabCtxMenu = new ComboList();
	m_tabCtxModel = new MyListModel(this);
	m_tabCtxModel->m_flags |= Qt::ItemIsEditable;
	m_tabCtxMenu->ui->filterView->setModel(m_tabCtxModel);
	m_tabCtxMenu->ui->filterView->setResizeMode(QListView::Adjust);
	m_tabCtxMenu->ui->filterView->setDropIndicatorShown(true);
	m_tabCtxMenu->ui->filterView->setMovement(QListView::Snap);
	m_tabCtxMenu->ui->filterView->setDragDropMode(QAbstractItemView::InternalMove);
	//m_tabCtxMenu->ui->filterView->setEditTriggers(QAbstractItemView::AllEditTriggers);
	m_tabCtxMenu->ui->filterView->setEditTriggers(QAbstractItemView::CurrentChanged);
	m_delegate = new ComboBoxDelegate(m_tabCtxMenu);
	m_tabCtxMenu->ui->filterView->setItemDelegate(m_delegate);
	m_tabCtxMenu->setVisible(0);

	connect(m_tabCtxMenu->ui->addButton, SIGNAL(clicked()), this, SLOT(onCtxAddButton()));
	connect(m_tabCtxMenu->ui->rmButton, SIGNAL(clicked()), this, SLOT(onCtxRmButton()));
	connect(m_tabCtxMenu->ui->commitButton, SIGNAL(clicked()), this, SLOT(onCtxCommitButton()));

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
	if (m_tabCtxModel && m_tabCtxModel->hasChildren())
		m_tabCtxModel->removeRows(0, m_tabCtxModel->rowCount());
}


void FilterMgr::setConfigToUI ()
{
	clearUI();
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
	{
		//QComboBox * cbx = new QComboBox(m_tabCtxMenu);
		//fillComboBoxWithFilters(cbx);
		//cbx->setCurrentIndex(m_filters[i]->type());
		if (m_filters[i])
		{
			QStandardItem * const qitem = new QStandardItem(m_filters[i]->typeName());
			qitem->setCheckable(true);
			qitem->setCheckState(m_filters[i]->enabled() ? Qt::Checked : Qt::Unchecked);
			m_tabCtxModel->appendRow(qitem);
		}
	}
}

void FilterMgr::onCtxAddButton ()
{
	QComboBox * cbx = new QComboBox(m_tabCtxMenu);
	fillComboBoxWithFilters(cbx);

	QStandardItem * const qitem = new QStandardItem(g_filterNames[1]);
	qitem->setCheckable(true);
	qitem->setCheckState(Qt::Checked);
	m_tabCtxModel->appendRow(qitem);
}

void FilterMgr::onCtxRmButton ()
{
	QModelIndexList const idxs = m_tabCtxMenu->ui->filterView->selectionModel()->selectedIndexes();
	foreach (QModelIndex index, idxs)
	{
		m_tabCtxModel->removeRow(index.row());
	}
}

void FilterMgr::onCtxCommitButton ()
{
	setUIToConfig();
	applyConfig();
}

void FilterMgr::setUIToConfig ()
{
	m_filter_order.clear();
	m_filter_order.reserve(e_filtertype_max_value);

	for (int i = 0, ie = m_tabCtxModel->rowCount(); i < ie; ++i)
	{
		QStandardItem const * const qitem = m_tabCtxModel->item(i, 0);
		if (qitem)
		{
			QString const & s = qitem->text();
			m_filter_order.push_back(s);
		}
	}
	recreateFilters();

	//std::vector<FilterBase *> m_origs;
}

void FilterMgr::onShowContextMenu (QPoint const & pt)
{
	setConfigToUI();

	bool const visible = m_tabCtxMenu->isVisible();
	m_tabCtxMenu->setVisible(!visible);

	if (m_tabCtxMenu->isVisible())
	{
		QPoint global_pos = mapToGlobal(pt);
		m_tabCtxMenu->move(global_pos);
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

void FilterMgr::onFilterEnabledChanged ()
{
	bool const some_enabled = someFilterEnabled();
	if (m_enabled ^ some_enabled)
	{
		m_enabled = some_enabled;
		emit filterEnabledChanged();
		qDebug("%s signal filterEnabledChanged", __FUNCTION__);
	}
	else
	{
		emitFilterChangedSignal();
	}
}


