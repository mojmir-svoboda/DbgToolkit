#include "filter_regex.h"

FilterRegex::FilterRegex (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterRegex)
{
}

void FilterRegex::initUI ()
{
	m_ui->setupUi(this);
}

void FilterRegex::doneUI ()
{
}

bool FilterRegex::accept (DecodedCommand const & cmd) const
{
	return true;
}

void FilterRegex::loadConfig (QString const & path)
{
}

void FilterRegex::saveConfig (QString const & path)
{
	//QString const fsname = fname + "." + g_filterStateTag;
	//saveFilterRegex(m_filter_state, fsname.toStdString());
}

void FilterRegex::applyConfig ()
{
	//m_filter_state.m_filtered_regexps = src.m_filtered_regexps;
}

///////////////////
bool FilterRegex::isMatchedRegexExcluded (QString str) const
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex const & fr = m_filtered_regexps.at(i);
		if (fr.exactMatch(str))
		{
			if (!fr.m_is_enabled)
				return false;
			else
			{
				return fr.m_state ? false : true;
			}
		}
	}
	return false;
}
void FilterRegex::setRegexInclusive (QString const & s, bool state)
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_filtered_regexps[i];
		if (fr.m_regex_str == s)
		{
			fr.m_state = state;
		}
	}
}
void FilterRegex::setRegexChecked (QString const & s, bool checked)
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_filtered_regexps[i];
		if (fr.m_regex_str == s)
		{
			fr.m_is_enabled = checked;
		}
	}
}
void FilterRegex::removeFromRegexFilters (QString const & s)
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_filtered_regexps[i];
		if (fr.m_regex_str == s)
		{
			m_filtered_regexps.removeAt(i);
			return;
		}
	}
}
void FilterRegex::appendToRegexFilters (QString const & s, bool enabled, bool state)
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
		if (m_filtered_regexps[i].m_regex_str == s)
			return;
	m_filtered_regexps.push_back(FilteredRegex(s, enabled, state));
}


