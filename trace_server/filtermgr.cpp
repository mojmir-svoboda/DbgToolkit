#include "filtermgr.h"
#include <algorithm>
#include <QGridLayout>
#include <QComboBox>
#include "ui_combolist.h"

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
	for (size_t i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->loadConfig(path);
}

void FilterMgr::saveConfig (QString const & path)
{
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

	void fillComboBoxWithFilters (QComboBox * cbx)
	{
		for (size_t i = 0; i < e_filtertype_max_value; ++i)
			cbx->addItem(g_filterNames[i]);
	}

void FilterMgr::setConfigToUI ()
{
	clearUI();
	for (size_t i = 0, ie = m_filters.size(); i < ie; ++i)
	{
		QComboBox * cbx = new QComboBox(m_tabCtxMenu);
		fillComboBoxWithFilters(cbx);
		cbx->setCurrentIndex(m_filters[i]->type());
	}
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
