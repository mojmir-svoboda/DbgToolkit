#pragma once
#include "types.h"
#include "history.h"

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
	QList<QString> m_preset_names;				/// registered presets
	QString m_last_search;
	History<QString> m_search_history;

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
	{ }

	void loadSearchHistory ();
	void saveSearchHistory () const;
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
		ar & m_tid;
		ar & m_from;
		ar & m_to;
		ar & m_file;
		ar & m_line;
	}
};

struct FilteredRegex {
	std::string m_regex_str;
	QRegExp m_regex;
	bool m_is_enabled;
	bool m_is_inclusive;

	bool isValid () const { return m_regex.isValid(); }
	bool exactMatch (QString str) const { return m_regex.exactMatch(str); }

	FilteredRegex () { }
	FilteredRegex (std::string const & rs, bool is_inclusive)
        : m_regex_str(rs), m_regex(QString::fromStdString(rs)), m_is_enabled(0), m_is_inclusive(is_inclusive)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & m_regex_str;
		ar & m_regex;
		ar & m_is_enabled;
		ar & m_is_inclusive;
	}
};

struct ColorizedText {
	E_ColorRole m_role;
	QColor m_qcolor;
	QColor m_bgcolor;
	std::string m_regex_str;
	QRegExp m_regex;
	bool m_is_enabled;

	bool isValid () const { return m_regex.isValid(); }
	bool exactMatch (QString str) const { return m_regex.exactMatch(str); }

	ColorizedText () { }

	ColorizedText (std::string const & rs, E_ColorRole r)
        : m_role(r)
        , m_qcolor(Qt::magenta), m_regex_str(rs), m_regex(QString::fromStdString(rs)), m_is_enabled(0)
	{ }

	ColorizedText (std::string const & rs, QColor const & col, E_ColorRole r)
        : m_role(r)
        , m_qcolor(col), m_regex_str(rs), m_regex(QString::fromStdString(rs)), m_is_enabled(0)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & m_role;
		ar & m_regex_str;
		ar & m_qcolor;
		ar & m_bgcolor;
		ar & m_regex_str;
		ar & m_regex;
		ar & m_is_enabled;
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
		ar & m_level_str;
		ar & m_level;
		ar & m_is_enabled;
		ar & m_state;
	}
};

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
		ar & m_ctx_str;
		ar & m_ctx;
		ar & m_is_enabled;
		ar & m_state;
	}
};

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
		ar & m_state;
		ar & m_collapsed;
	}
};


