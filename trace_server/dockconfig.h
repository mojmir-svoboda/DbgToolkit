#pragma once
#include <QString>
#include <QVector>
#include <QColor>
#include "constants.h"
#include <filters/file_filter.hpp>
#include "types.h"
#include "dockedwidget.h"

struct DockConfig
{
	QString 		m_tag;
	QString 		m_font;
	int 			m_fontsize;
	int 			m_row_width;
	QVector<int> 	m_columns_sizes;		/// column sizes for each registered application
	bool 			m_show;
	typedef tree_filter<DockedInfo> data_filters_t;
	data_filters_t	m_docked_widgets_data;

	DockConfig (QString const & tag)
		: m_tag(tag)
		, m_font("Verdana")
		, m_fontsize(10)
		, m_row_width(18)
		, m_show(true)
		, m_docked_widgets_data()
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("tag", m_tag);
		ar & boost::serialization::make_nvp("font", m_font);
		ar & boost::serialization::make_nvp("fontsize", m_fontsize);
		ar & boost::serialization::make_nvp("row_width", m_row_width);
		ar & boost::serialization::make_nvp("columns_sizes", m_columns_sizes);
		ar & boost::serialization::make_nvp("show", m_show);
		ar & boost::serialization::make_nvp("docked_widgets_data", m_docked_widgets_data);
	}

	void clear ()
	{
		m_docked_widgets_data.clear();
		DockConfig rhs(g_traceServerName);
		*this = rhs;
		rhs.m_docked_widgets_data.root = 0;
	}
};

bool loadConfig (DockConfig & config, QString const & fname);
bool saveConfig (DockConfig const & config, QString const & fname);
void fillDefaultConfig (DockConfig & config);

