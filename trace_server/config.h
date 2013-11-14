#pragma once
#include "types.h"
#include "history.h"
#include <boost/serialization/nvp.hpp>

struct GlobalConfig {
	unsigned m_hotkey;
	bool m_hidden;
	bool m_was_maximized;
	bool m_dump_mode;
	QList<QString> m_app_names;					/// registered applications
	QList<columns_setup_t> m_columns_setup;		/// column setup for each registered application
	QList<columns_sizes_t> m_columns_sizes;		/// column sizes for each registered application
	QList<columns_align_t> m_columns_align;		/// column align for each registered application
	QList<columns_elide_t> m_columns_elide;		/// column elide for each registered application
	QList<QColor> m_thread_colors;				/// predefined coloring of threads
	QList<QString> m_registry_pnames;			/// legacy preset names from registry
	QString m_last_search;
	History<QString> m_search_history;
	History<QString> m_preset_history;

	QString m_trace_addr;
	unsigned short m_trace_port;
	QString m_profiler_addr;
	unsigned short m_profiler_port;
	QString m_appdir;

	GlobalConfig ()
		: m_hotkey(0x91 /*VK_SCROLL*/)
		, m_hidden(false)
		, m_was_maximized(false)
		, m_dump_mode(false)
		, m_search_history(16)
		, m_preset_history(16)
	{ }

	void loadHistory ();
	void saveHistory () const;
};

struct CollapsedBlock {
	QString m_tid;
	int m_from;
	int m_to;
	QString m_file;
	QString m_line;

	CollapsedBlock () { }

	CollapsedBlock (QString tid, int from, int to, QString file, QString line)
		: m_tid(tid), m_from(from), m_to(to), m_file(file), m_line(line)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("tid", m_tid);
		ar & boost::serialization::make_nvp("from", m_from);
		ar & boost::serialization::make_nvp("to", m_to);
		ar & boost::serialization::make_nvp("file", m_file);
		ar & boost::serialization::make_nvp("line", m_line);
	}
};

struct FilteredRegex {
	QString m_regex_str;
	QRegExp m_regex;
	bool m_is_enabled;
	int m_state;

	bool isValid () const { return m_regex.isValid(); }
	bool exactMatch (QString str) const { return m_regex.exactMatch(str); }

	FilteredRegex () { }
	FilteredRegex (QString const & rs, bool enabled, int state)
        : m_regex_str(rs), m_regex(rs), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("regex_str", m_regex_str);
		ar & boost::serialization::make_nvp("regex", m_regex);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

struct FilteredString {
	QString m_string;
	bool m_is_enabled;
	int m_state;

	bool match (QString const & str) const { return str.contains(m_string, Qt::CaseInsensitive); }

	FilteredString () { }
	FilteredString (QString const & s, bool enabled, int state)
        : m_string(s), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("string", m_string);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

struct ColorizedText {
	E_ColorRole m_role;
	QColor m_qcolor;
	QColor m_bgcolor;
	QString m_regex_str;
	QRegExp m_regex;
	bool m_is_enabled;

	bool isValid () const { return m_regex.isValid(); }
	bool exactMatch (QString str) const { return m_regex.exactMatch(str); }

	ColorizedText () { }

	ColorizedText (QString const & rs, E_ColorRole r)
        : m_role(r)
        , m_qcolor(Qt::magenta), m_regex_str(rs), m_regex(rs), m_is_enabled(0)
	{ }

	ColorizedText (QString const & rs, QColor const & col, E_ColorRole r)
        : m_role(r)
        , m_qcolor(col), m_regex_str(rs), m_regex(rs), m_is_enabled(0)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("role", m_role);
		ar & boost::serialization::make_nvp("regex_str", m_regex_str);
		ar & boost::serialization::make_nvp("qcolor", m_qcolor);
		ar & boost::serialization::make_nvp("bgcolor", m_bgcolor);
		ar & boost::serialization::make_nvp("regex_str", m_regex_str);
		ar & boost::serialization::make_nvp("regex", m_regex);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
	}
};

struct FilteredLevel {
	QString m_level_str;
	int m_level;
	bool m_is_enabled;
	int m_state;

	FilteredLevel () { }
	FilteredLevel (QString level, bool enabled, int state)
        : m_level_str(level), m_level(level.toInt()), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("level_str", m_level_str);
		ar & boost::serialization::make_nvp("level", m_level);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

inline bool operator< (FilteredLevel const & lhs, FilteredLevel const & rhs)
{
	return lhs.m_level < rhs.m_level;
}

struct FilteredContext {
	QString m_ctx_str;
	unsigned long long m_ctx;
	bool m_is_enabled;
	int m_state;

	FilteredContext () { }
	FilteredContext (QString ctx, bool enabled, int state)
        : m_ctx_str(ctx), m_ctx(ctx.toULongLong()), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("ctx_str", m_ctx_str);
		ar & boost::serialization::make_nvp("ctx", m_ctx);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

inline bool operator< (FilteredContext const & lhs, FilteredContext const & rhs)
{
	return lhs.m_ctx < rhs.m_ctx;
}

struct TreeModelItem {
	/*@member	state
	 * duplicates qt enum
	 *	Qt::Unchecked	0	The item is unchecked.
	 *	Qt::PartiallyChecked	1	The item is partially checked. Items in hierarchical models may be partially checked if some, but not all, of their children are checked.
	 *	Qt::Checked	2	The item is checked.
	 */
	int m_state;
	int m_collapsed;

	TreeModelItem () : m_state(e_Unchecked), m_collapsed(true) { }
	TreeModelItem (int s) : m_state(s), m_collapsed(true) { }
	TreeModelItem (int s, bool c) : m_state(s), m_collapsed(c) { }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("state", m_state);
		ar & boost::serialization::make_nvp("collapsed", m_collapsed);
	}
};

struct FilteredScript {
	QString m_name;
	QString m_script_str;
	QString m_path;
	bool m_is_enabled;
	int m_state;

	//bool match (QString const & str) const { return str.contains(m_string, Qt::CaseInsensitive); }

	FilteredScript () { }
	FilteredScript (QString const & name, bool enabled, int state)
		: m_name(name), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("name", m_name);
		ar & boost::serialization::make_nvp("path", m_path);
		ar & boost::serialization::make_nvp("script", m_script_str);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

