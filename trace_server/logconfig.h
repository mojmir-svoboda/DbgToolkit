#pragma once
#include "types.h"

namespace log {

	struct LogConfig
	{
		QString m_tag;
		QString m_title;
		int m_history_ln;
		bool m_show;
	/*unsigned m_hotkey;
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
	QString m_appdir;*/


		LogConfig ()
			: m_tag()
			, m_timer_delay_ms(50)
			, m_history_ln(128*128)
			, m_show(true)
		{ }

		LogConfig (QString const & tag)
			: m_tag(tag)
			, m_timer_delay_ms(50)
			, m_history_ln(128*128)
			, m_show(true)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("title", m_title);
			ar & boost::serialization::make_nvp("gfcfg", m_gvcfg);
			ar & boost::serialization::make_nvp("timer", m_timer_delay_ms);
			ar & boost::serialization::make_nvp("length", m_history_ln);
			ar & boost::serialization::make_nvp("show", m_show);
		}
	};

	bool loadConfig (LogConfig & config, QString const & fname);
	bool saveConfig (LogConfig const & config, QString const & fname);
}

