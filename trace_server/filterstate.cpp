#include "filterstate.h"
#include <tlv_parser/tlv_encoder.h>

FilterState::FilterState (QObject * parent)
{
	qDebug("FilterState()");
	m_colorized_texts.push_back(ColorizedText(".*[Ww]arning.*", QColor(Qt::yellow), e_Bg));
	m_colorized_texts.push_back(ColorizedText(".*[Ee]rror.*", QColor(Qt::red), e_Fg));
}

FilterState::~FilterState ()
{
	qDebug("~FilterState()");
}


void FilterState::clearFilters ()
{
	m_file_filters.clear();
	m_tid_filters.clear();
	m_lvl_filters.clear();
	m_filtered_regexps.clear();
	m_colorized_texts.clear();
	m_collapse_blocks.clear();
}


///////// file filters
bool FilterState::isFileLinePresent (fileline_t const & item, TreeModelItem & fi) const
{
	TreeModelItem const * tmp_fi = 0;
	bool const exists = m_file_filters.is_present(item.first + "/" + item.second, tmp_fi);
	if (exists)
		fi = *tmp_fi;
	return exists;
}
bool FilterState::isFileLinePresent (QString const & fileline, TreeModelItem & fi) const
{
	TreeModelItem const * tmp_fi = 0;
	bool const exists = m_file_filters.is_present(fileline, tmp_fi);
	if (exists)
		fi = *tmp_fi;
	return exists;
}

///////// ctx filters
bool FilterState::isCtxPresent (QString const & item, bool & enabled) const
{
	for (int i = 0, ie = m_ctx_filters.size(); i < ie; ++i)
		if (m_ctx_filters.at(i).m_ctx_str == item)
		{
			FilteredContext const & fc = m_ctx_filters.at(i);
			enabled = fc.m_is_enabled;
			return true;
		}
	return false;
}
void FilterState::appendCtxFilter (QString const & item)
{
	for (int i = 0, ie = m_ctx_filters.size(); i < ie; ++i)
		if (m_ctx_filters[i].m_ctx_str == item)
		{
			FilteredContext & fc = m_ctx_filters[i];
			fc.m_is_enabled = true;
			return;
		}
	m_ctx_filters.push_back(FilteredContext(item, true, 0));

}
void FilterState::removeCtxFilter (QString const & item)
{
	for (int i = 0, ie = m_ctx_filters.size(); i < ie; ++i)
		if (m_ctx_filters[i].m_ctx_str == item)
		{
			FilteredContext & fc = m_ctx_filters[i];
			fc.m_is_enabled = false;
			return;
		}
}

///////// tid filters
void FilterState::appendTIDFilter (QString const & item)
{
	m_tid_filters.push_back(item);
}
void FilterState::removeTIDFilter (QString const & item)
{
	m_tid_filters.erase(std::remove(m_tid_filters.begin(), m_tid_filters.end(), item), m_tid_filters.end());
}
bool FilterState::isTIDExcluded (QString const & item) const
{
	return std::find(m_tid_filters.begin(), m_tid_filters.end(), item) != m_tid_filters.end();
}

///////// lvl filters
void FilterState::appendLvlFilter (QString const & item)
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
void FilterState::removeLvlFilter (QString const & item)
{
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters[i].m_level_str == item)
		{
			FilteredLevel & l = m_lvl_filters[i];
			l.m_is_enabled = false;
			return;
		}
}
bool FilterState::isLvlPresent (QString const & item, bool & enabled, E_LevelMode & lvlmode) const
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
bool FilterState::setLvlMode (QString const & item, bool enabled, E_LevelMode lvlmode)
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
bool FilterState::isMatchedColorizedText (QString str, QColor & color, E_ColorRole & role) const
{
	for (int i = 0, ie = m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText const & ct = m_colorized_texts.at(i);
		if (ct.exactMatch(str))
		{
			color = ct.m_qcolor;
			role = ct.m_role;
			return ct.m_is_enabled;
		}
	}
	return false;
}
void FilterState::setRegexColor (QString const & s, QColor col)
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
}
void FilterState::removeFromColorRegexFilters (QString const & s)
{
	for (int i = 0, ie = m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText & ct = m_colorized_texts[i];
		if (ct.m_regex_str == s)
		{
			m_colorized_texts.removeAt(i);
			return;
		}
	}
}
void FilterState::appendToColorRegexFilters (QString const & s)
{
	for (int i = 0, ie = m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText & ct = m_colorized_texts[i];
		if (ct.m_regex_str == s)
			return;
	}
	m_colorized_texts.push_back(ColorizedText(s, e_Fg));
}

///////////////////
bool FilterState::isMatchedRegexExcluded (QString str) const
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
void FilterState::setRegexInclusive (QString const & s, bool state)
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
void FilterState::setRegexChecked (QString const & s, bool checked)
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
void FilterState::removeFromRegexFilters (QString const & s)
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
void FilterState::appendToRegexFilters (QString const & s, bool enabled, bool state)
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
		if (m_filtered_regexps[i].m_regex_str == s)
			return;
	m_filtered_regexps.push_back(FilteredRegex(s, enabled, state));
}


///////////////////
bool FilterState::isMatchedStringExcluded (QString str) const
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
void FilterState::setStringState (QString const & s, int state)
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
void FilterState::setStringChecked (QString const & s, bool checked)
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
void FilterState::removeFromStringFilters (QString const & s)
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
void FilterState::appendToStringFilters (QString const & s, bool enabled, int state)
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
		if (m_filtered_strings[i].m_string == s)
			return;
	m_filtered_strings.push_back(FilteredString(s, enabled, state));
}

void FilterState::merge_rhs (node_t * lhs, node_t const * rhs)
{
	node_t const * rhs_child = rhs->children;
	while (rhs_child)
	{
		node_t * lhs_child = lhs->children;
		bool found = false;
		while (lhs_child)
		{
			if (lhs_child->key == rhs_child->key)
			{
				found = true;
				break;
			}
			lhs_child = lhs_child->next;
		}

		if (!found)
		{
			lhs_child = new node_t(*rhs_child);
			node_t::node_append(lhs, lhs_child);
		}
		else
			lhs_child->data = rhs_child->data;

		merge(lhs_child, rhs_child);

		rhs_child = rhs_child->next;
	}
}

void FilterState::merge_state (node_t * lhs, node_t const * rhs)
{
	node_t * lhs_child = lhs->children;

	while (lhs_child)
	{
		node_t * rhs_child = rhs->children;
		bool found = false;
		while (rhs_child)
		{
			if (rhs_child->key == lhs_child->key)
			{
				found = true;
				// assert na state
				break;
			}
			rhs_child = rhs_child->next;
		}

		if (found)
		{
			lhs_child->data = rhs_child->data;
			merge_state(lhs_child, rhs_child);
		}
		else
		{
			node_t * parent = lhs_child->parent;
			if (parent)
			{
				switch (parent->data.m_state)
				{
					case e_Unchecked:
						m_file_filters.set_state_to_childs(lhs_child, parent->data);
						break;
					case e_PartialCheck:
						m_file_filters.set_state_to_childs(lhs_child, TreeModelItem(e_Unchecked, 1));
						break;
					case e_Checked:
						m_file_filters.set_state_to_childs(lhs_child, parent->data);
						break;
				}
			}
		}

		lhs_child = lhs_child->next;
	}
}

void FilterState::merge (node_t * lhs, node_t const * rhs)
{
	merge_rhs(lhs, rhs);
	merge_state(lhs, rhs);
}

void FilterState::merge_with (file_filters_t const & rhs)
{   
	node_t * const rhs_root = rhs.root;
	if (m_file_filters.root && rhs_root)
		m_file_filters.root->data = rhs_root->data;
	merge(m_file_filters.root, rhs_root);
} 









