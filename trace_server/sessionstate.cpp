#include "sessionstate.h"
#include <tlv_parser/tlv_encoder.h>

SessionState::SessionState (QObject * parent)
	: m_app_idx(-1)
	, m_storage_idx(-2)
	, m_tab_widget(0)
	, m_exclude_content_to_row(0)
	, m_toggle_ref_row(0)
	, m_filter_mode(e_Exclude)
	, m_columns_setup_current(0)
	, m_columns_setup_template(0)
	, m_columns_align_template(0)
	, m_columns_elide_template(0)
	, m_columns_sizes(0)
	, m_name()
	, m_recv_bytes(0)
{
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

void SessionState::setupColumns (QList<QString> const * cs_template, columns_sizes_t * sizes
			, columns_align_t const * ca_template, columns_elide_t const * ce_template)
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

void SessionState::insertColumn4Tag (tlv::tag_t tag, int column_idx)
{
	m_tags2columns.insert(tag, column_idx);
}
void SessionState::insertColumn ()
{
	m_columns_setup_current->push_back(QString());
	if (m_columns_sizes->size() < m_columns_setup_current->size())
		m_columns_sizes->push_back(127);
	qDebug("inserting column and size. tmpl_sz=%u curr_sz=%u sizes_sz=%u", m_columns_setup_template->size(), m_columns_setup_current->size(), m_columns_sizes->size());
}

int SessionState::insertColumn (tlv::tag_t tag)
{
	insertColumn();
	int const column_index = m_columns_setup_current->size() - 1;
	char const * name = tlv::get_tag_name(tag);
	insertColumn4Tag(tag, column_index);

	if (name)
		m_columns_setup_current->operator[](column_index) = QString::fromStdString(name);
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
bool SessionState::isFileLinePresent (std::string const & fileline, TreeModelItem & fi) const
{
	return m_file_filters.is_present(fileline, fi);
}

///////// ctx filters
bool SessionState::isCtxPresent (std::string const & item, bool & enabled) const
{
	QString const qitem = QString::fromStdString(item);
	for (int i = 0, ie = m_ctx_filters.size(); i < ie; ++i)
		if (m_ctx_filters.at(i).m_ctx_str == qitem)
		{
			FilteredContext const & fc = m_ctx_filters.at(i);
			enabled = fc.m_is_enabled;
			return true;
		}
	return false;
}
void SessionState::appendCtxFilter (std::string const & item)
{
	QString const qitem = QString::fromStdString(item);
	for (int i = 0, ie = m_ctx_filters.size(); i < ie; ++i)
		if (m_ctx_filters[i].m_ctx_str == qitem)
		{
			FilteredContext & fc = m_ctx_filters[i];
			fc.m_is_enabled = true;
			return;
		}
	m_ctx_filters.push_back(FilteredContext(qitem, true, 0));

}
void SessionState::removeCtxFilter (std::string const & item)
{
	QString const qitem = QString::fromStdString(item);
	for (int i = 0, ie = m_ctx_filters.size(); i < ie; ++i)
		if (m_ctx_filters[i].m_ctx_str == qitem)
		{
			FilteredContext & fc = m_ctx_filters[i];
			fc.m_is_enabled = false;
			return;
		}
}

///////// tid filters
void SessionState::appendTIDFilter (std::string const & item)
{
	m_tid_filters.push_back(item);
}
void SessionState::removeTIDFilter (std::string const & item)
{
	m_tid_filters.erase(std::remove(m_tid_filters.begin(), m_tid_filters.end(), item), m_tid_filters.end());
}
bool SessionState::isTIDExcluded (std::string const & item) const
{
	return std::find(m_tid_filters.begin(), m_tid_filters.end(), item) != m_tid_filters.end();
}

///////// lvl filters
void SessionState::appendLvlFilter (std::string const & item)
{
	QString const qitem = QString::fromStdString(item);
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters[i].m_level_str == qitem)
		{
			FilteredLevel & l = m_lvl_filters[i];
			l.m_is_enabled = true;
			return;
		}
	m_lvl_filters.push_back(FilteredLevel(qitem, true, e_LvlInclude));
}
void SessionState::removeLvlFilter (std::string const & item)
{
	QString const qitem = QString::fromStdString(item);
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters[i].m_level_str == qitem)
		{
			FilteredLevel & l = m_lvl_filters[i];
			l.m_is_enabled = false;
			return;
		}
}
bool SessionState::isLvlPresent (std::string const & item, bool & enabled, E_LevelMode & lvlmode) const
{
	QString const qitem = QString::fromStdString(item);
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters.at(i).m_level_str == qitem)
		{
			FilteredLevel const & l = m_lvl_filters.at(i);
			lvlmode = static_cast<E_LevelMode>(l.m_state);
			enabled = l.m_is_enabled;
			return true;
		}
	return false;
}
bool SessionState::setLvlMode (std::string const & item, bool enabled, E_LevelMode lvlmode)
{
	QString const qitem = QString::fromStdString(item);
	for (int i = 0, ie = m_lvl_filters.size(); i < ie; ++i)
		if (m_lvl_filters.at(i).m_level_str == qitem)
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
void SessionState::setRegexColor (std::string const & s, QColor col)
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
void SessionState::setColorRegexChecked (std::string const & s, bool checked)
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
void SessionState::removeFromColorRegexFilters (std::string const & s)
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
void SessionState::appendToColorRegexFilters (std::string const & s)
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
				return fr.m_is_inclusive ? false : true;
			}
		}
	}
	return false;
}
void SessionState::setRegexInclusive (std::string const & s, bool inclusive)
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex & fr = m_filtered_regexps[i];
		if (fr.m_regex_str == s)
		{
			fr.m_is_inclusive = inclusive;
		}
	}
}
void SessionState::setRegexChecked (std::string const & s, bool checked)
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
void SessionState::removeFromRegexFilters (std::string const & s)
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
void SessionState::appendToRegexFilters (std::string const & s, bool enabled, bool inclusive)
{
	for (int i = 0, ie = m_filtered_regexps.size(); i < ie; ++i)
		if (m_filtered_regexps[i].m_regex_str == s)
			return;
	m_filtered_regexps.push_back(FilteredRegex(s, enabled));
	m_filtered_regexps.back().m_is_inclusive = inclusive;
}


