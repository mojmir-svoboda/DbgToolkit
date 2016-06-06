#pragma once
#include <QString>
#include <QColor>
#include <QVector>
#include <dock/dockedconfig.h>
#include <widgets/findconfig.h>
#include <widgets/colorizeconfig.h>

namespace table {

	struct TableConfig : DockedConfigBase
	{
		QString m_tag;
		QString m_title;
		std::vector<QString> m_hhdr;
		std::vector<int> m_hsize;
		std::vector<QString> m_vhdr;
		std::vector<int> m_vsize;

		bool m_sparse_table;
		bool m_filtering_enabled;
		FindConfig m_find_config;
		ColorizeConfig m_colorize_config;

		TableConfig ()
			: m_tag()
			, m_title()
			, m_sparse_table(false)
			, m_filtering_enabled(true)
		{
	/*		m_hhdr.reserve(32);
			m_hsize.reserve(32);
			m_vhdr.reserve(32);
			m_vsize.reserve(32);*/
		}

		TableConfig (QString const & tag)
			: m_tag(tag)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			DockedConfigBase::serialize(ar, version);

			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("hhdr", m_hhdr);
			ar & boost::serialization::make_nvp("hsz", m_hsize);
			ar & boost::serialization::make_nvp("vhdr", m_vhdr);
			ar & boost::serialization::make_nvp("vsz", m_vsize);
			ar & boost::serialization::make_nvp("sparse_table", m_sparse_table);
			ar & boost::serialization::make_nvp("filtering_enabled", m_filtering_enabled);
			ar & boost::serialization::make_nvp("find_config", m_find_config);
			ar & boost::serialization::make_nvp("colorize_config", m_colorize_config);
			//ar & boost::serialization::make_nvp("central_widget", m_central_widget);
		}

		void clear ()
		{
			*this = TableConfig();
		}
	};

	bool loadConfig (TableConfig & config, QString const & fname);
	bool saveConfig (TableConfig const & config, QString const & fname);
	void fillDefaultConfig (TableConfig & config);
}

