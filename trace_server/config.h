#pragma once
#include "types.h"
#include "history.h"

struct GlobalConfig
{
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
	{ }
};

namespace plot {

	struct CurveConfig
	{
		QString m_tag;
		int m_line_width;
		int m_style;
		QColor m_color;

		CurveConfig ()
			: m_line_width(2)
			, m_style(0)
			, m_color(Qt::red)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & m_tag;
			ar & m_line_width;
			ar & m_style;
			ar & m_color;
		}
	};

	struct AxisConfig
	{
		QString m_label;

		AxisConfig () { }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & m_label;
		}
	};

	struct PlotConfig
	{
		QString m_tag;
		QList<CurveConfig> m_ccfg;
		QList<AxisConfig> m_acfg;

		int m_timer_delay_ms;
		int m_history_ln;
		int m_from;
		// qwt state
		// flags
		bool m_auto_scroll;
		bool m_unused_b0;
		bool m_unused_b1;
		bool m_unused_b2;

		PlotConfig ()
			: m_tag()
			, m_timer_delay_ms(50)
			, m_history_ln(256)
			, m_from(0)
			, m_auto_scroll(true)
		{ }

		PlotConfig (QString const & tag)
			: m_tag(tag)
			, m_timer_delay_ms(50)
			, m_history_ln(256)
			, m_from(0)
			, m_auto_scroll(true)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & m_tag;
			ar & m_ccfg;
			ar & m_acfg;
			ar & m_timer_delay_ms;
			ar & m_history_ln;
			//ar & m_from;
			// flags
			ar & m_auto_scroll;
			ar & m_unused_b0;
			ar & m_unused_b1;
			ar & m_unused_b2;
		}
	};

	bool loadConfig (PlotConfig & config, QString const & fname);
	bool saveConfig (PlotConfig & config, QString const & fname);
}

