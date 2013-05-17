#pragma once
#include "types.h"
#include <filters/file_filter.hpp>

namespace profiler {

	struct TagInfo
	{
		/*@member	state
		 * duplicates qt enum
		 *	Qt::Unchecked	0	The item is unchecked.
		 *	Qt::PartiallyChecked	1	The item is partially checked. Items in hierarchical models may be partially checked if some, but not all, of their children are checked.
		 *	Qt::Checked	2	The item is checked.
		 */
		int m_state;
		int m_collapsed;

		TagInfo () : m_state(e_Unchecked), m_collapsed(true) { }
		TagInfo (int s) : m_state(s), m_collapsed(true) { }
		TagInfo (int s, bool c) : m_state(s), m_collapsed(c) { }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("state", m_state);
			ar & boost::serialization::make_nvp("collapsed", m_collapsed);
		}
	};

	struct SessionState {

		typedef tree_filter<TagInfo> tag_filters_t;
		tag_filters_t m_tag_filters;

		explicit SessionState (QObject *parent = 0)
		{ }

		~SessionState ()
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag_filters", m_tag_filters);
		}

		tag_filters_t const & getTagFilters () const { return m_tag_filters; }

		/*bool isTagPresent (QString const & tag, TagInfo & fi) const
		{
			return m_tag_filters.is_present(tag, fi);
		}*/
	};
}

