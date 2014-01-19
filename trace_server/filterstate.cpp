#include "filterstate.h"
#include <tlv_parser/tlv_encoder.h>

FilterState::FilterState (QObject * parent)
{
	qDebug("FilterState()");
}

FilterState::~FilterState ()
{
	qDebug("~FilterState()");
}


void FilterState::clearFilters ()
{
	//m_colorized_texts.clear();
	m_collapse_blocks.clear();
}


///////// collapsed scopes
void FilterState::appendCollapsedBlock (QString tid, int from, int to, QString file, QString line)
{
	m_collapse_blocks.push_back(CollapsedBlock(tid, from, to, file, line));
}
bool FilterState::findCollapsedBlock (QString tid, int from, int to) const
{
	for (int i = 0, ie = m_collapse_blocks.size(); i < ie; ++i)
	{
		CollapsedBlock const & b = m_collapse_blocks.at(i);
		if (b.m_tid == tid && b.m_from == from && to == b.m_to)
			return true;
	}
	return false;
}
bool FilterState::eraseCollapsedBlock (QString tid, int from, int to)
{
	for (int i = 0, ie = m_collapse_blocks.size(); i < ie; ++i)
	{
		CollapsedBlock const & b = m_collapse_blocks.at(i);
		if (b.m_tid == tid && b.m_from == from && to == b.m_to)
		{
			m_collapse_blocks.removeAt(i);
			return true;
		}
	}
	return false;
}
bool FilterState::isBlockCollapsed (QString tid, int row) const
{
	for (int i = 0, ie = m_collapse_blocks.size(); i < ie; ++i)
	{
		CollapsedBlock const & b = m_collapse_blocks.at(i);
		if (b.m_tid == tid)
		{
			if (b.m_from < row && row < b.m_to)
				return true;
		}
	}
	return false;
}
bool FilterState::isBlockCollapsedIncl (QString tid, int row) const
{
	for (int i = 0, ie = m_collapse_blocks.size(); i < ie; ++i)
	{
		CollapsedBlock const & b = m_collapse_blocks.at(i);
		if (b.m_tid == tid)
		{
			if (b.m_from <= row && row <= b.m_to)
				return true;
		}
	}
	return false;
}

///////// color filters
/*bool FilterState::isMatchedColorizedText (QString str, QColor & fgcolor, QColor & bgcolor) const
{
	for (int i = 0, ie = m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText const & ct = m_colorized_texts.at(i);
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
/*void FilterState::setRegexColor (QString const & s, QColor col)
{
	for (int i = 0, ie = m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText & ct = m_colorized_texts[i];
		if (ct.m_regex_str == s)
		{
			ct.m_qcolor = col;
		}
	}
}
void FilterState::setColorRegexChecked (QString const & s, bool checked)
{
	for (int i = 0, ie = m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText & ct = m_colorized_texts[i];
		if (ct.m_regex_str == s)
		{
			ct.m_is_enabled = checked;
		}
	}
}*/





