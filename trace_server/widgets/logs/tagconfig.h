#pragma once
#include "logfile_protocol.h"

namespace logs {

	struct TagDesc
	{
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
			/*char const * name = tlv::get_tag_name(tag);
			if (name)
			m_tag_str = name;
			else
			m_tag_str = QString("Tag_") + QString::number(tag);*/
		}

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("tag_str", m_tag_str);
			ar & boost::serialization::make_nvp("align", m_align);
			ar & boost::serialization::make_nvp("align_str", m_align_str);
			ar & boost::serialization::make_nvp("elide", m_elide);
			ar & boost::serialization::make_nvp("elide_str", m_elide_str);
			ar & boost::serialization::make_nvp("size", m_size);
		}
	};

	struct TagConfig {

		QVector<TagDesc> m_tag_desc;

		TagConfig ();

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag_desc", m_tag_desc);
		}

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

		TagDesc const & findTag (int tag) const
		{
			Q_ASSERT(tag < m_tag_desc.size());
			TagDesc const & ref = m_tag_desc[tag];
			return ref;
		}

		void setupTag (int tag, TagDesc & desc);
	};

	inline TagConfig::TagConfig ()
	{
		m_tag_desc.reserve(proto::e_max_tag_count + 16);
		m_tag_desc.push_back(TagDesc(proto::tag_stime, e_AlignRight, e_ElideLeft, 80));
		m_tag_desc.push_back(TagDesc(proto::tag_ctime, e_AlignRight, e_ElideLeft, 80));
		m_tag_desc.push_back(TagDesc(proto::tag_dt, e_AlignRight, e_ElideLeft, 64));
		m_tag_desc.push_back(TagDesc(proto::tag_tid, e_AlignRight, e_ElideLeft, 16));
		m_tag_desc.push_back(TagDesc(proto::tag_lvl, e_AlignRight, e_ElideLeft, 16));
		m_tag_desc.push_back(TagDesc(proto::tag_ctx, e_AlignRight, e_ElideLeft, 16));
		m_tag_desc.push_back(TagDesc(proto::tag_file, e_AlignRight, e_ElideLeft, 192));
		m_tag_desc.push_back(TagDesc(proto::tag_line, e_AlignRight, e_ElideNone, 32));
		m_tag_desc.push_back(TagDesc(proto::tag_func, e_AlignRight, e_ElideLeft, 128));
		m_tag_desc.push_back(TagDesc(proto::tag_msg, e_AlignLeft, e_ElideRight, 512));
	}

	inline void TagConfig::setupTag (int tag, TagDesc & desc)
	{
		desc.m_tag = tag;
		// 	char const * name = tlv::get_tag_name(tag);
		// 	if (name)
		// 		desc.m_tag_str = name;
		// 	else
		// 		desc.m_tag_str = QString::number(tag);
		desc.m_align = e_AlignLeft;
		desc.m_elide = e_ElideRight;
		desc.m_size = 64;
	}

}