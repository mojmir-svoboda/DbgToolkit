#pragma once
#include "types.h"
#include <QVector>
#include <tlv_parser/tlv_parser.h>

enum E_Align { e_AlignLeft, e_AlignRight, E_AlignMid, e_max_align_enum_value };
typedef char T_Aligns[e_max_align_enum_value];
static T_Aligns aligns = { 'L', 'R', 'M' };
static char const * alignsStr[e_max_align_enum_value] = { "Left", "Right", "Middle" };
inline char alignToString (E_Align a)
{
	return a < e_max_align_enum_value ? aligns[a] : aligns[e_AlignLeft];
}
inline E_Align stringToAlign (char c) {
	for (size_t i = 0; i < e_max_align_enum_value; ++i)
		if (aligns[i] == c)
			return static_cast<E_Align>(i);
	return e_AlignLeft; // default
}

enum E_Elide { e_ElideLeft = 0, e_ElideRight, e_ElideMiddle, e_ElideNone, e_max_elide_enum_value }; // synchronized with Qt::TextElideMode
typedef char T_Elides[e_max_elide_enum_value];
static T_Elides elides = { 'L', 'R', 'M', '-' };
static char const * elidesStr[e_max_elide_enum_value] = { "Left", "Right", "Middle", "-" };

inline char elideToString (E_Elide e)
{
	return e < e_max_elide_enum_value ? elides[e] : elides[e_ElideRight];
}
inline E_Elide stringToElide (char c) {
	for (size_t i = 0; i < e_max_elide_enum_value; ++i)
		if (elides[i] == c)
			return static_cast<E_Elide>(i);
	return e_ElideLeft; // default
}


struct TagDesc {
	int		m_tag;
	QString m_tag_str;
	E_Align m_align;
	QString m_align_str;
	E_Elide m_elide;
	QString m_elide_str;
	int	    m_size;

	TagDesc (int tag = -1, E_Align align = e_AlignLeft, E_Elide elide = e_ElideRight, int size = 0)
		: m_tag(tag), m_align(align), m_align_str(alignToString(align)), m_elide(elide), m_elide_str(elideToString(elide)), m_size(size)
	{
		char const * name = tlv::get_tag_name(tag);
		if (name)
			m_tag_str = name;
		else
			m_tag_str = QString("Tag_") + QString::number(tag);
	}
};

struct TagConfig {

	QVector<TagDesc> m_tag_desc;

	TagConfig ();

	TagDesc & findOrCreateTag (int tag)
	{
		if (tag >= m_tag_desc.size())
		{
			m_tag_desc.resize(tag + 1);
		}
		
		TagDesc & ref = m_tag_desc[tag];
		if (ref.m_tag >= 0)
			return ref;
		else
		{
			setupTag(tag, ref);
			return ref;
		}
	}

	void setupTag (int tag, TagDesc & desc);
};


inline TagConfig::TagConfig ()
{
	m_tag_desc.reserve(tlv::tag_max_value + 16);
	m_tag_desc.push_back(TagDesc( tlv::tag_invalid ));
	m_tag_desc.push_back(TagDesc( tlv::tag_app     ));
	m_tag_desc.push_back(TagDesc( tlv::tag_pid     ));
	m_tag_desc.push_back(TagDesc( tlv::tag_time, e_AlignRight, e_ElideLeft,   64 ));
	m_tag_desc.push_back(TagDesc( tlv::tag_tid,  e_AlignRight, e_ElideLeft,   16 ));
	m_tag_desc.push_back(TagDesc( tlv::tag_file, e_AlignRight, e_ElideLeft,  192 ));
	m_tag_desc.push_back(TagDesc( tlv::tag_line, e_AlignRight, e_ElideNone,   32 ));
	m_tag_desc.push_back(TagDesc( tlv::tag_func, e_AlignRight, e_ElideLeft,  128 ));
	m_tag_desc.push_back(TagDesc( tlv::tag_msg,  e_AlignLeft,  e_ElideRight, 512 ));
	m_tag_desc.push_back(TagDesc( tlv::tag_lvl,  e_AlignRight, e_ElideLeft,   16 ));
	m_tag_desc.push_back(TagDesc( tlv::tag_ctx,  e_AlignRight, e_ElideLeft,   16 ));
	m_tag_desc.push_back(TagDesc( tlv::tag_dt,   e_AlignRight, e_ElideLeft,   64 ));
	m_tag_desc.push_back(TagDesc( tlv::tag_bool ));
	m_tag_desc.push_back(TagDesc( tlv::tag_int ));
	m_tag_desc.push_back(TagDesc( tlv::tag_string ));
	m_tag_desc.push_back(TagDesc( tlv::tag_float ));
	m_tag_desc.push_back(TagDesc( tlv::tag_x ));
	m_tag_desc.push_back(TagDesc( tlv::tag_y ));
	m_tag_desc.push_back(TagDesc( tlv::tag_z ));
}

inline void TagConfig::setupTag (int tag, TagDesc & desc)
{
	desc.m_tag = tag;
	char const * name = tlv::get_tag_name(tag);
	if (name)
		desc.m_tag_str = name;
	else
		desc.m_tag_str = QString::number(tag);
	desc.m_align = e_AlignLeft;
	desc.m_elide = e_ElideRight;
	desc.m_size = 64;
}

