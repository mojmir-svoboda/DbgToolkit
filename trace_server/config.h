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


