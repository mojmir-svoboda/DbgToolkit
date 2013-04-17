#pragma once
#include "types.h"

namespace gantt {

	struct GraphConfig
	{
		QString m_tag;
		QString m_label;
		int m_sync_group;
		QColor m_fgcolor;
		QColor m_bgcolor;
		QColor m_symbolcolor;
		int m_symbolsize;
		bool m_show;
		bool m_auto_scroll;

		GraphConfig ()
			: m_sync_group(0)
			, m_fgcolor(Qt::yellow)
			, m_bgcolor(Qt::black)
			, m_symbolcolor(Qt::red)
			, m_symbolsize(6)
			, m_show(true)
			, m_auto_scroll(true)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("label", m_label);
			ar & boost::serialization::make_nvp("sync_group", m_sync_group);
			ar & boost::serialization::make_nvp("fgcol", m_fgcolor);
			ar & boost::serialization::make_nvp("bgcol", m_bgcolor);
			ar & boost::serialization::make_nvp("symcol", m_symbolcolor);
			ar & boost::serialization::make_nvp("symsize", m_symbolsize);
			ar & boost::serialization::make_nvp("show", m_show);
			ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
		}
	};

	struct GanttConfig
	{
		QString m_tag;
		QString m_title;
		QList<GraphConfig> m_gfcfg;
		int m_timer_delay_ms;
		int m_history_ln;
		bool m_show;

		GanttConfig ()
			: m_tag()
			, m_timer_delay_ms(50)
			, m_history_ln(2048)
			, m_show(true)
		{ }

		GanttConfig (QString const & tag)
			: m_tag(tag)
			, m_timer_delay_ms(50)
			, m_history_ln(2048)
			, m_show(true)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("title", m_title);
			ar & boost::serialization::make_nvp("gfcfg", m_gfcfg);
			ar & boost::serialization::make_nvp("timer", m_timer_delay_ms);
			ar & boost::serialization::make_nvp("length", m_history_ln);
			ar & boost::serialization::make_nvp("show", m_show);
		}

		bool findGraphConfig (QString const & tag, GraphConfig const * & ccfg)
		{
			for (int i = 0, ie = m_gfcfg.size(); i < ie; ++i)
				if (m_gfcfg.at(i).m_tag == tag)
				{
					ccfg = &m_gfcfg.at(i);
					return true;
				}
			return false;
		}
	};

	bool loadConfig (GanttConfig & config, QString const & fname);
	bool saveConfig (GanttConfig const & config, QString const & fname);
}

