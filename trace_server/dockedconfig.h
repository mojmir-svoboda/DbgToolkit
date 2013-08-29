#pragma once

struct DockedConfigBase {

	// font
	// font size
	bool m_auto_scroll;
	bool m_show;
	bool m_central_widget;
	int  m_sync_group;

	DockedConfigBase () : m_auto_scroll(false), m_show(true), m_central_widget(false), m_sync_group(0) { }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
		ar & boost::serialization::make_nvp("show", m_show);
		ar & boost::serialization::make_nvp("sync_group", m_sync_group);
		ar & boost::serialization::make_nvp("central_widget", m_central_widget);
		//ar & boost::serialization::make_nvp("font", m_font);
		//ar & boost::serialization::make_nvp("fontsize", m_fontsize);
	}

};



