#include "filter_tid.h"

FilterTid::FilterTid (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterTid)
{
}

void FilterTid::initUI ()
{
	m_ui->setupUi(this);
}

void FilterTid::doneUI ()
{
}

bool FilterTid::accept (DecodedCommand const & cmd) const
{
	return true;
}

void FilterTid::loadConfig (QString const & path)
{
}

void FilterTid::saveConfig (QString const & path)
{
	//QString const fsname = fname + "." + g_filterStateTag;
	//saveFilterTid(m_filter_state, fsname.toStdString());
}

void FilterTid::applyConfig ()
{
	//m_filter_state.merge_with(src.m_file_filters);
}


///////// tid filters
void FilterTid::appendTIDFilter (QString const & item)
{
	m_tid_filters.push_back(item);
}
void FilterTid::removeTIDFilter (QString const & item)
{
	m_tid_filters.erase(std::remove(m_tid_filters.begin(), m_tid_filters.end(), item), m_tid_filters.end());
}
bool FilterTid::isTIDExcluded (QString const & item) const
{
	return std::find(m_tid_filters.begin(), m_tid_filters.end(), item) != m_tid_filters.end();
}


