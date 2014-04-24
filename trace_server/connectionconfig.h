#pragma once
#include "types.h"
#include "history.h"

struct ConnectionConfig
{
	bool	  m_dump_mode;
	bool	  m_auto_scroll;
	bool	  m_buffered;
	bool	  m_show;
	int		  m_level;
	QString	  m_time_units_str;
	float	  m_time_units;
	QString	  m_font;
	int		  m_fontsize;
	History<QString> m_preset_history;

	void loadHistory ();
	void saveHistory () const;

	ConnectionConfig ()
		: m_dump_mode(false)
		, m_auto_scroll(false)
		, m_buffered(true)
		, m_show(true)
		, m_level(3)
		, m_time_units_str("ms")
		, m_time_units(stringToUnitsValue(m_time_units_str))
		, m_font("Verdana")
		, m_fontsize(10)
		, m_preset_history(16)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("dump_mode", m_dump_mode);
		ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
		ar & boost::serialization::make_nvp("buffered", m_buffered);
		ar & boost::serialization::make_nvp("show", m_show);
		ar & boost::serialization::make_nvp("level", m_level);
		ar & boost::serialization::make_nvp("time_units_str", m_time_units);
		ar & boost::serialization::make_nvp("time_units", m_time_units);
		ar & boost::serialization::make_nvp("font", m_font);
		ar & boost::serialization::make_nvp("fontsize", m_fontsize);
	}
};

