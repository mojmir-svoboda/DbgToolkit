#include "filter_lvl.h"

FilterLvl::FilterLvl (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterLvl)
{
	//setupModelFile();
}

void FilterLvl::initUI ()
{
	m_ui->setupUi(this);
}

void FilterLvl::doneUI ()
{
	//destroyModelFile();
}

bool FilterLvl::accept (DecodedCommand const & cmd) const
{
	return true;
}

void FilterLvl::loadConfig (QString const & path)
{
}

void FilterLvl::saveConfig (QString const & path)
{
	//QString const fsname = fname + "." + g_filterStateTag;
	//saveFilterLvl(m_filter_state, fsname.toStdString());
}

void FilterLvl::applyConfig ()
{
	//m_lvl_filters = src.m_lvl_filters;
}

///////// lvl filters
void FilterLvl::appendLvlFilter (QString const & item)
{
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters[i].m_level_str == item)
		{
			FilteredLevel & l = m_lvl_filters[i];
			l.m_is_enabled = true;
			return;
		}
	m_lvl_filters.push_back(FilteredLevel(item, true, e_LvlInclude));
	std::sort(m_lvl_filters.begin(), m_lvl_filters.end());
}
void FilterLvl::removeLvlFilter (QString const & item)
{
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters[i].m_level_str == item)
		{
			FilteredLevel & l = m_lvl_filters[i];
			l.m_is_enabled = false;
			return;
		}
}
bool FilterLvl::isLvlPresent (QString const & item, bool & enabled, E_LevelMode & lvlmode) const
{
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters.at(i).m_level_str == item)
		{
			FilteredLevel const & l = m_lvl_filters.at(i);
			lvlmode = static_cast<E_LevelMode>(l.m_state);
			enabled = l.m_is_enabled;
			return true;
		}
	return false;
}
bool FilterLvl::setLvlMode (QString const & item, bool enabled, E_LevelMode lvlmode)
{
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters.at(i).m_level_str == item)
		{
			FilteredLevel & l = m_lvl_filters[i];
			l.m_state = lvlmode;
			l.m_is_enabled = enabled;
			return true;
		}
	return false;

}

