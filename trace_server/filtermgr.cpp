#include "filtermgr.h"
#include <algorithm>
#include <QGridLayout>

FilterMgr::FilterMgr (QWidget * parent)
	: FilterBase(parent)
{
	m_filters.reserve(e_filtertype_max_value);
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

void FilterMgr::addFilter (FilterBase * b) { m_filters.push_back(b); }
void FilterMgr::rmFilter (FilterBase * b) { m_filters.erase(std::remove(m_filters.begin(), m_filters.end(), b), m_filters.end()); }

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
	m_widget = m_widget;
}

void FilterMgr::doneUI ()
{
}

