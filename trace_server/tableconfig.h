#pragma once
#include "types.h"

namespace table {

	struct TableConfig
	{
		QString m_tag;
		QString m_title;
		QList<QString> m_hhdr;
		QList<int> m_hsize;
		QList<QString> m_vhdr;
		QList<int> m_vsize;

		bool m_auto_scroll;
		bool m_show;
		bool m_unused_b1;
		bool m_unused_b2;

		TableConfig ()
			: m_tag()
			, m_title()
			, m_auto_scroll(true)
			, m_show(true)
			, m_unused_b1(false)
			, m_unused_b2(false)
		{
			m_hhdr.reserve(32);
			m_hsize.reserve(32);
			m_vhdr.reserve(32);
			m_vsize.reserve(32);
		}

		TableConfig (QString const & tag)
			: m_tag(tag)
			, m_auto_scroll(true)
			, m_show(true)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("hhdr", m_hhdr);
			ar & boost::serialization::make_nvp("hsz", m_hsize);
			ar & boost::serialization::make_nvp("vhdr", m_vhdr);
			ar & boost::serialization::make_nvp("vsz", m_vsize);
			// flags
			ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
			ar & boost::serialization::make_nvp("show", m_show);
			ar & boost::serialization::make_nvp("flag1", m_unused_b1);
			ar & boost::serialization::make_nvp("flag2", m_unused_b2);
		}
	};

	bool loadConfig (TableConfig & config, QString const & fname);
	bool saveConfig (TableConfig const & config, QString const & fname);
}

