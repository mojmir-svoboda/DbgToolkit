#pragma once
#include "types.h"
#include "frameviewconfig.h"

//namespace gantt {

	struct FrameViewConfig
	{
		QString m_tag;
		QString m_title;
		QList<FrameViewConfig> m_gvcfg;
		int m_timer_delay_ms;
		int m_history_ln;
		bool m_show;

		FrameViewConfig ()
			: m_tag()
			, m_timer_delay_ms(50)
			, m_history_ln(2048)
			, m_show(true)
		{ }

		FrameViewConfig (QString const & tag)
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
			ar & boost::serialization::make_nvp("gfcfg", m_gvcfg);
			ar & boost::serialization::make_nvp("timer", m_timer_delay_ms);
			ar & boost::serialization::make_nvp("length", m_history_ln);
			ar & boost::serialization::make_nvp("show", m_show);
		}

		bool findFrameViewConfig (QString const & tag, FrameViewConfig const * & ccfg)
		{
			for (int i = 0, ie = m_gvcfg.size(); i < ie; ++i)
				if (m_gvcfg.at(i).m_tag == tag)
				{
					ccfg = &m_gvcfg.at(i);
					return true;
				}
			return false;
		}
	};

	bool loadConfig (FrameViewConfig & config, QString const & fname);
	bool saveConfig (FrameViewConfig const & config, QString const & fname);
//}

