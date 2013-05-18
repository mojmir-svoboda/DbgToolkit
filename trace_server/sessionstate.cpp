#include "sessionstate.h"
#include <tlv_parser/tlv_encoder.h>
#include "settings.h"

SessionState::SessionState (QObject * parent)
	: m_app_idx(-1)
	, m_storage_idx(-2)
	, m_exclude_content_to_row(0)
	, m_time_ref_row(0)
	, m_current_tag(-1)
	, m_current_selection(-1)
	, m_time_ref_value(0)
	, m_columns_setup_current(0)
	, m_columns_setup_template(0)
	, m_columns_align_template(0)
	, m_columns_elide_template(0)
	, m_columns_sizes(0)
	, m_app_name()
	, m_recv_bytes(0)
	, m_csv_separator("")
{
	qDebug("SessionState()");
	m_colorized_texts.push_back(ColorizedText(".*[Ww]arning.*", QColor(Qt::yellow), e_Bg));
	m_colorized_texts.push_back(ColorizedText(".*[Ee]rror.*", QColor(Qt::red), e_Fg));
	static int counter = 0;
	m_storage_idx = counter;
	++counter;
}

SessionState::~SessionState ()
{
	qDebug("~SessionState()");
	if (m_columns_setup_current)
		delete m_columns_setup_current;
}

void SessionState::setupColumns (QList<QString> * cs_template, columns_sizes_t * sizes
			, columns_align_t * ca_template, columns_elide_t * ce_template)
{
	m_columns_sizes = sizes;
	m_columns_setup_template = cs_template;
	m_columns_align_template = ca_template;
	m_columns_elide_template = ce_template;

	if (!m_columns_setup_current)
	{
		m_columns_setup_current = new QList<QString>();
	}
	else
	{
		m_columns_setup_current->clear();
	}

	m_tags2columns.clear();
	*m_columns_setup_current = *m_columns_setup_template;
	for (size_t i = 0, ie = cs_template->size(); i < ie; ++i)
	{
		size_t const tag_idx = tlv::tag_for_name(cs_template->at(i).toStdString().c_str());
		if (tag_idx != tlv::tag_invalid)
		{
			m_tags2columns.insert(tag_idx, static_cast<int>(i)); // column index is int in Qt toolkit
			//qDebug("SessionState::setupColumns col[%u] tag_idx=%u tag_name=%s", i, tag_idx, cs->at(i).toStdString().c_str());
		}
	}
}

void SessionState::setupColumnsCSV (QList<QString> * cs_template, columns_sizes_t * sizes
			, columns_align_t * ca_template, columns_elide_t * ce_template)
{
	m_columns_sizes = sizes;
	m_columns_setup_template = cs_template;
	m_columns_align_template = ca_template;
	m_columns_elide_template = ce_template;

	if (!m_columns_setup_current)
	{
		m_columns_setup_current = new QList<QString>();
	}
	else
	{
		m_columns_setup_current->clear();
	}

	m_tags2columns.clear();
	*m_columns_setup_current = *m_columns_setup_template;
}

void SessionState::setupThreadColors (QList<QColor> const & tc)
{
	m_thread_colors = tc;
}

int SessionState::findColumn4Tag (tlv::tag_t tag) const
{
	QMap<tlv::tag_t, int>::const_iterator it = m_tags2columns.find(tag);
	if (it != m_tags2columns.end())
		return it.value();
	return -1;
}

int SessionState::findColumn4TagInTemplate (tlv::tag_t tag) const
{
	QMap<tlv::tag_t, int>::const_iterator it = m_tags2columns.find(tag);
	if (it != m_tags2columns.end())
		return it.value();
	return -1;
}

int SessionState::insertColumn (tlv::tag_t tag)
{
	m_columns_setup_current->push_back(QString());
	tlv::tag_t const used_tag = tag > tlv::tag_max_value ? tlv::tag_time : tag;
	if (m_columns_sizes->size() < m_columns_setup_current->size())
	{
		m_columns_align_template->push_back(QString(alignToString(default_aligns[used_tag])));
		m_columns_sizes->push_back(default_sizes[used_tag]);
		m_columns_elide_template->push_back(QString(elideToString(default_elides[used_tag])));
	}

	qDebug("inserting column and size. tmpl_sz=%u curr_sz=%u sizes_sz=%u", m_columns_setup_template->size(), m_columns_setup_current->size(), m_columns_sizes->size());

	int const column_index = m_columns_setup_current->size() - 1;
	char const * name = tlv::get_tag_name(tag);

	m_tags2columns.insert(tag, column_index);

	if (name)
		m_columns_setup_current->operator[](column_index) = name;
	else
		m_columns_setup_current->operator[](column_index) = QString("???");

	return column_index;
}

void SessionState::clearFilters ()
{
	m_file_filters.clear();
	m_tid_filters.clear();
	m_lvl_filters.clear();
	m_filtered_regexps.clear();
	m_colorized_texts.clear();
	m_collapse_blocks.clear();
}


///////// file filters
bool SessionState::isFileLinePresent (fileline_t const & item, TreeModelItem & fi) const
{
	TreeModelItem const * tmp_fi = 0;
	bool const exists = m_file_filters.is_present(item.first + "/" + item.second, tmp_fi);
	if (exists)
		fi = *tmp_fi;
	return exists;
}
bool SessionState::isFileLinePresent (QString const & fileline, TreeModelItem & fi) const
{
	TreeModelItem const * tmp_fi = 0;
	bool const exists = m_file_filters.is_present(fileline, tmp_fi);
	if (exists)
		fi = *tmp_fi;
	return exists;
}

///////// ctx filters
bool SessionState::isCtxPresent (QString const & item, bool & enabled) const
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
void SessionState::appendCtxFilter (QString const & item)
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
void SessionState::removeCtxFilter (QString const & item)
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
void SessionState::appendTIDFilter (QString const & item)
{
	m_tid_filters.push_back(item);
}
void SessionState::removeTIDFilter (QString const & item)
{
	m_tid_filters.erase(std::remove(m_tid_filters.begin(), m_tid_filters.end(), item), m_tid_filters.end());
}
bool SessionState::isTIDExcluded (QString const & item) const
{
	return std::find(m_tid_filters.begin(), m_tid_filters.end(), item) != m_tid_filters.end();
}

///////// lvl filters
void SessionState::appendLvlFilter (QString const & item)
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
void SessionState::removeLvlFilter (QString const & item)
{
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters[i].m_level_str == item)
		{
			FilteredLevel & l = m_lvl_filters[i];
			l.m_is_enabled = false;
			return;
		}
}
bool SessionState::isLvlPresent (QString const & item, bool & enabled, E_LevelMode & lvlmode) const
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
bool SessionState::setLvlMode (QString const & item, bool enabled, E_LevelMode lvlmode)
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
void SessionState::appendCollapsedBlock (QString tid, int from, int to, QString file, QString line)
{
	m_collapse_blocks.push_back(CollapsedBlock(tid, from, to, file, line));
}
bool SessionState::findCollapsedBlock (QString tid, int from, int to) const
{
	for (int i = 0, ie = m_collapse_blocks.size(); i < ie; ++i)
	{
		CollapsedBlock const & b = m_collapse_blocks.at(i);
		if (b.m_tid == tid && b.m_from == from && to == b.m_to)
			return true;
	}
	return false;
}
bool SessionState::eraseCollapsedBlock (QString tid, int from, int to)
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
bool SessionState::isBlockCollapsed (QString tid, int row) const
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
bool SessionState::isBlockCollapsedIncl (QString tid, int row) const
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
bool SessionState::isMatchedColorizedText (QString str, QColor & color, E_ColorRole & role) const
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
void SessionState::setRegexColor (QString const & s, QColor col)
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
void SessionState::setColorRegexChecked (QString const & s, bool checked)
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
void SessionState::removeFromColorRegexFilters (QString const & s)
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
void SessionState::appendToColorRegexFilters (QString const & s)
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
bool SessionState::isMatchedRegexExcluded (QString str) const
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
void SessionState::setRegexInclusive (QString const & s, bool state)
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
void SessionState::setRegexChecked (QString const & s, bool checked)
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
void SessionState::removeFromRegexFilters (QString const & s)
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
void SessionState::appendToRegexFilters (QString const & s, bool enabled, bool state)
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
		if (m_filtered_regexps[i].m_regex_str == s)
			return;
	m_filtered_regexps.push_back(FilteredRegex(s, enabled, state));
}


///////////////////
bool SessionState::isMatchedStringExcluded (QString str) const
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
void SessionState::setStringState (QString const & s, int state)
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
void SessionState::setStringChecked (QString const & s, bool checked)
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
void SessionState::removeFromStringFilters (QString const & s)
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
void SessionState::appendToStringFilters (QString const & s, bool enabled, int state)
{
	for (int i = 0, ie = m_filtered_strings.size(); i < ie; ++i)
		if (m_filtered_strings[i].m_string == s)
			return;
	m_filtered_strings.push_back(FilteredString(s, enabled, state));
}

void SessionState::merge_rhs (node_t * lhs, node_t const * rhs)
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

void SessionState::merge_state (node_t * lhs, node_t const * rhs)
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

void SessionState::merge (node_t * lhs, node_t const * rhs)
{
	merge_rhs(lhs, rhs);
	merge_state(lhs, rhs);
}

void SessionState::merge_with (file_filters_t const & rhs)
{   
	node_t * const rhs_root = rhs.root;
	if (m_file_filters.root && rhs_root)
		m_file_filters.root->data = rhs_root->data;
	merge(m_file_filters.root, rhs_root);
} 

void SessionState::addColorTagRow (int row)
{
	for (int i = 0, ie = m_color_tag_rows.size(); i < ie; ++i)
		if (m_color_tag_rows.at(i) == row)
		{
			removeColorTagRow(row);
			return;
		}
	m_color_tag_rows.push_back(row);
}

bool SessionState::findColorTagRow (int row) const
{
	for (int i = 0, ie = m_color_tag_rows.size(); i < ie; ++i)
		if (m_color_tag_rows.at(i) == row)
			return true;
	return false;
}

void SessionState::removeColorTagRow (int row)
{
	m_color_tag_rows.erase(std::remove(m_color_tag_rows.begin(), m_color_tag_rows.end(), row), m_color_tag_rows.end());
}
