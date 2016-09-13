#pragma once
#include "types.h"
#include "constants.h"
#include <utils/history.h>
#include <QDir>
#include "mixerconfig.h"
#include "widgets/quickstringconfig.h"
#include "widgets/findconfig.h"

struct GlobalConfig {
	unsigned  m_hotkey { 0x91 /*VK_SCROLL*/ };
	bool	  m_hidden { false };
	bool	  m_was_maximized { false };
	bool	  m_dump_mode  { false };
	bool	  m_auto_scroll  { false };
	bool	  m_buffered  { false };
	bool	  m_on_top  { false };
	int		  m_level  { 3 };
	MixerConfig m_mixer;
	int		  m_logs_recv_level { 2 };
	int		  m_plots_recv_level { 0 };
	int		  m_tables_recv_level { 0 };
	int		  m_gantts_recv_level { 0 };
	QString	  m_time_units_str { "ms" };
	float	  m_time_units { stringToUnitsValue(m_time_units_str) };
	QString	  m_font { "Verdana" };
	int		  m_fontsize { 10 };
	QString	  m_trace_addr { "127.0.0.1" };
	unsigned short m_trace_port { g_defaultPort };
	QString	  m_profiler_addr { "127.0.0.1" };
	unsigned short m_profiler_port { 13147 };
	QString	  m_appdir { QDir::homePath() + "/" + g_traceServerDirName };

	History<QString> m_preset_history { 24 };
	History<QString> m_recent_history { 24 };
	FindConfig m_find_config;
	QuickStringConfig m_quick_string_config;

	GlobalConfig ()	{ }

	void loadHistory (QString const & path);
	void saveHistory (QString const & path) const;

	void fillDefaultConfig ()
	{
		*this = GlobalConfig();
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("hotkey", m_hotkey);
		ar & boost::serialization::make_nvp("hidden", m_hidden);
		ar & boost::serialization::make_nvp("was_maximized", m_was_maximized);
		ar & boost::serialization::make_nvp("dump_mode", m_dump_mode);
		ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
		ar & boost::serialization::make_nvp("buffered", m_buffered);
		ar & boost::serialization::make_nvp("on_top", m_on_top);
		ar & boost::serialization::make_nvp("level", m_level);
		ar & boost::serialization::make_nvp("mixer", m_mixer);
		ar & boost::serialization::make_nvp("m_logs_recv_level", m_logs_recv_level);
		ar & boost::serialization::make_nvp("m_plots_recv_level", m_plots_recv_level);
		ar & boost::serialization::make_nvp("m_tables_recv_level", m_tables_recv_level);
		ar & boost::serialization::make_nvp("m_gantts_recv_level", m_gantts_recv_level);
		ar & boost::serialization::make_nvp("time_units_str", m_time_units_str);
		ar & boost::serialization::make_nvp("time_units", m_time_units);
		ar & boost::serialization::make_nvp("font", m_font);
		ar & boost::serialization::make_nvp("fontsize", m_fontsize);

		ar & boost::serialization::make_nvp("trace_addr", m_trace_addr);
		ar & boost::serialization::make_nvp("trace_port", m_trace_port);
		ar & boost::serialization::make_nvp("profiler_addr", m_profiler_addr);
		ar & boost::serialization::make_nvp("profiler_port", m_profiler_port);
		if (version > 0)
		{
			ar & boost::serialization::make_nvp("quick_string", m_quick_string_config);
		}
		if (version > 1)
		{
			ar & boost::serialization::make_nvp("find", m_find_config);
		}
		//ar & boost::serialization::make_nvp("appdir", m_appdir); // do not want this probably
	}
};
BOOST_CLASS_VERSION(GlobalConfig, 2)

struct TreeModelItem {
	/*@member	state
	 * duplicates qt enum
	 *	Qt::Unchecked	0	The item is unchecked.
	 *	Qt::PartiallyChecked	1	The item is partially checked. Items in hierarchical models may be partially checked if some, but not all, of their children are checked.
	 *	Qt::Checked	2	The item is checked.
	 */
	int m_state;
	int m_collapsed;

	TreeModelItem () : m_state(0), m_collapsed(true) { }
	TreeModelItem (int s) : m_state(s), m_collapsed(true) { }
	TreeModelItem (int s, bool c) : m_state(s), m_collapsed(c) { }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("state", m_state);
		ar & boost::serialization::make_nvp("collapsed", m_collapsed);
	}
};


