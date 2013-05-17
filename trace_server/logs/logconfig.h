#pragma once
#include <QString>
#include <QVector>
#include <QColor>

namespace logs {

	struct LogConfig
	{
		QString m_tag;
		int m_history_ln;
		int m_sync_group;
		QString m_font;
		int m_fontsize;
		QVector<QString> m_columns_setup;		/// column setup for each registered application
		QVector<int> m_columns_sizes;		/// column sizes for each registered application
		QVector<QString> m_columns_align;		/// column align for each registered application
		QVector<QString> m_columns_elide;		/// column elide for each registered application
		QVector<QColor> m_thread_colors;				/// predefined coloring of threads
		bool m_show;
		bool m_auto_scroll;

		LogConfig ()
			: m_tag()
			, m_history_ln(128*128)
			, m_sync_group(0)
			, m_font("Verdana")
			, m_fontsize(10)
			, m_show(true)
			, m_auto_scroll(true)
		{ }

		LogConfig (QString const & tag)
			: m_tag(tag)
			, m_history_ln(128*128)
			, m_sync_group(0)
			, m_font("Verdana")
			, m_fontsize(10)
			, m_show(true)
			, m_auto_scroll(true)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("length", m_history_ln);
			ar & boost::serialization::make_nvp("sync_group", m_sync_group);
			ar & boost::serialization::make_nvp("font", m_font);
			ar & boost::serialization::make_nvp("fontsize", m_fontsize);
			ar & boost::serialization::make_nvp("columns_setup", m_columns_setup);
			ar & boost::serialization::make_nvp("columns_sizes", m_columns_sizes);
			ar & boost::serialization::make_nvp("columns_align", m_columns_align);
			ar & boost::serialization::make_nvp("columns_elide", m_columns_elide);
			ar & boost::serialization::make_nvp("thread_colors", m_thread_colors);
			ar & boost::serialization::make_nvp("show", m_show);
			ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
		}
	};

	bool loadConfig (LogConfig & config, QString const & fname);
	bool saveConfig (LogConfig const & config, QString const & fname);
}

