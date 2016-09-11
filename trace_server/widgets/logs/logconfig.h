#pragma once
#include <QString>
#include <QVector>
#include <QColor>
#include <dock/dockedconfig.h>
#include <constants.h>
#include <widgets/findconfig.h>
#include <widgets/quickstringconfig.h>
#include <widgets/colorizeconfig.h>
#include <boost/serialization/version.hpp>
#include <serialize/ser_qt.h>
#include "tagconfig.h"

namespace logs {

	struct LogConfig : DockedConfigBase
	{
		QString m_tag;
		int m_history_ln { 128 * 128 };
		QString m_font { "Verdana" };
		int m_fontsize { 10 };
		int m_row_width { 18 };
		int m_indent_level { 2 };
		int m_cut_path_level { 2 };
		int m_cut_namespace_level { 3 };
		int m_curr_tooltab { 1 };
		TagConfig m_tag_config;
		std::vector<QString> 	m_columns_setup;		/// column setup for each registered application
		std::vector<int> 		m_columns_sizes;		/// column sizes for each registered application
		std::vector<QString> 	m_columns_align;		/// column align for each registered application
		std::vector<QString> 	m_columns_elide;		/// column elide for each registered application
		std::vector<std::pair<QColor, QColor>> 	m_thread_colors;		/// predefined coloring of threads
		QByteArray m_header_setup_stream;
		bool m_in_view { true };
		bool m_filtering { true };
		bool m_clr_filters { true };
		bool m_scopes_enabled { true };
		bool m_dt_scopes_enabled { true };
		bool m_indent { true };
		bool m_cut_path { true };
		bool m_cut_namespaces { true };
		bool m_dt_enabled { false };
		bool m_dt_colorize { false };
		bool m_filter_proxy { false };
		bool m_find_proxy { false };
		bool m_simplify_strings { false };
		bool m_unquote_strings { false };
		bool m_csv_has_header { false };
		QString m_csv_separator;
		FindConfig m_find_config;
		QuickStringConfig m_quick_string_config;
		ColorizeConfig m_colorize_config;

		LogConfig () { }

		LogConfig (QString const & tag)
			: LogConfig()
		{
			m_tag = tag;
		}

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			DockedConfigBase::serialize(ar, version);
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("history_ln", m_history_ln);
			ar & boost::serialization::make_nvp("font", m_font);
			ar & boost::serialization::make_nvp("fontsize", m_fontsize);
			ar & boost::serialization::make_nvp("row_width", m_row_width);
			ar & boost::serialization::make_nvp("indent_level", m_indent_level);
			ar & boost::serialization::make_nvp("cut_path_level", m_cut_path_level);
			ar & boost::serialization::make_nvp("cut_namespace_level", m_cut_namespace_level);
			ar & boost::serialization::make_nvp("curr_tooltab", m_curr_tooltab);
			ar & boost::serialization::make_nvp("tag_config", m_tag_config);
			ar & boost::serialization::make_nvp("columns_setup", m_columns_setup);
			ar & boost::serialization::make_nvp("columns_sizes", m_columns_sizes);
			ar & boost::serialization::make_nvp("columns_align", m_columns_align);
			ar & boost::serialization::make_nvp("columns_elide", m_columns_elide);
			ar & boost::serialization::make_nvp("thread_colors", m_thread_colors);
			//ar & boost::serialization::make_nvp("storage_order", m_storage_order);
			ar & boost::serialization::make_nvp("header_setup_stream", m_header_setup_stream);
			ar & boost::serialization::make_nvp("in_view", m_in_view);
			ar & boost::serialization::make_nvp("filtering", m_filtering);
			ar & boost::serialization::make_nvp("clr_filters", m_clr_filters);
			ar & boost::serialization::make_nvp("scopes", m_scopes_enabled);
			ar & boost::serialization::make_nvp("dt_scopes", m_dt_scopes_enabled);
			ar & boost::serialization::make_nvp("indent", m_indent);
			ar & boost::serialization::make_nvp("cut_path", m_cut_path);
			ar & boost::serialization::make_nvp("cut_namespaces", m_cut_namespaces);
			ar & boost::serialization::make_nvp("dt_enabled", m_dt_enabled);
			if (version > 1)
				ar & boost::serialization::make_nvp("dt_colorize", m_dt_colorize);
			ar & boost::serialization::make_nvp("find_config", m_find_config);
			if (version > 0)
				ar & boost::serialization::make_nvp("quick_string_config", m_quick_string_config);
			ar & boost::serialization::make_nvp("simplify_strings", m_simplify_strings);
			ar & boost::serialization::make_nvp("unquote_strings", m_unquote_strings);
			ar & boost::serialization::make_nvp("csv_has_header", m_csv_has_header);
			ar & boost::serialization::make_nvp("csv_separator", m_csv_separator);
			ar & boost::serialization::make_nvp("colorize_config", m_colorize_config);
		}

		void clear ()
		{
			*this = LogConfig();
		}

		void addTagToConfig (proto::tags tag)
		{
			TagDesc const & td = m_tag_config.findOrCreateTag(tag);
			m_columns_setup.push_back(proto::get_tag_name(td.m_tag));
			m_columns_sizes.push_back(td.m_size);
			m_columns_align.push_back(td.m_align_str);
			m_columns_elide.push_back(td.m_elide_str);
		}
	};


	bool loadConfig (LogConfig & config, QString const & fname);
	bool saveConfig (LogConfig const & config, QString const & fname);
	void fillDefaultConfig (LogConfig & config);
	bool validateConfig (logs::LogConfig const & cfg);
}

BOOST_CLASS_VERSION(logs::LogConfig, 2)
