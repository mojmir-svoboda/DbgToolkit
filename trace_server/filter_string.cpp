#include "filter_string.h"

FilterString::FilterString (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterString)
{
}

void FilterString::initUI ()
{
	m_ui->setupUi(this);
}

void FilterString::doneUI ()
{
}

bool FilterString::accept (DecodedCommand const & cmd) const
{
	return true;
}

void FilterString::loadConfig (QString const & path)
{
}

void FilterString::saveConfig (QString const & path)
{
	//QString const fsname = fname + "." + g_filterStateTag;
	//saveFilterString(m_filter_state, fsname.toStdString());
}

void FilterString::applyConfig ()
{
}

///////////////////
bool FilterString::isMatchedStringExcluded (QString str) const
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
	{
		FilteredString const & fr = m_filtered_strings.at(i);
		if (fr.match(str))
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
void FilterString::setStringState (QString const & s, int state)
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
	{
		FilteredString & fr = m_filtered_strings[i];
		if (fr.m_string == s)
		{
			fr.m_state = state;
		}
	}
}
void FilterString::setStringChecked (QString const & s, bool checked)
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
	{
		FilteredString & fr = m_filtered_strings[i];
		if (fr.m_string == s)
		{
			fr.m_is_enabled = checked;
		}
	}
}
void FilterString::removeFromStringFilters (QString const & s)
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
	{
		FilteredString & fr = m_filtered_strings[i];
		if (fr.m_string == s)
		{
			m_filtered_strings.removeAt(i);
			return;
		}
	}
}
void FilterString::appendToStringFilters (QString const & s, bool enabled, int state)
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
		if (m_filtered_strings[i].m_string == s)
			return;
	m_filtered_strings.push_back(FilteredString(s, enabled, state));
}






