#pragma once
#include <vector>
#include <memory>
#include <boost/call_traits.hpp>
#include <functional>
#include <algorithm>
#include "hash.h"

template <typename KeyT>
struct ItemInfo
{
	unsigned m_use_count;
	unsigned m_hash;
	KeyT m_key;

	ItemInfo () : m_use_count(0), m_hash(0) { }

	ItemInfo (typename boost::call_traits<KeyT>::param_type key)
		: m_use_count(0), m_hash(hash<std::string>()(key)), m_key(key) { }

	/*friend bool operator< (ItemInfo const & lhs, ItemInfo const & rhs) { return lhs.m_use_count < rhs.m_use_count; }*/

	friend bool operator== (ItemInfo const & lhs, ItemInfo const & rhs)
	{
		return lhs.m_key == rhs.m_key;
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & m_use_count;
		ar & m_hash;
		ar & m_key;
	}
};

template <typename KeyT>
struct History
{
	unsigned m_key_limit;
	typedef KeyT key_t;
	typedef std::vector<ItemInfo<key_t> > dict_t;
	typedef typename dict_t::iterator it_t;
	typedef typename dict_t::const_iterator const_it_t;
	dict_t m_dict;

	History (unsigned item_limit)
		: m_key_limit(item_limit)
	{ 
		m_dict.reserve(m_key_limit);
	}

	void insert (typename boost::call_traits<KeyT>::param_type key)
	{
		unsigned const key_hash = hash<std::string>()(key);
		for (size_t i = 0, ie = m_dict.size(); i < ie; ++i)
		{
			if (m_dict[i].m_hash == key_hash && m_dict[i].m_key == key)
			{
				++m_dict[i].m_use_count;
				return;
			}
		}

		// item not found, replace another
		if (m_dict.size() >= m_key_limit)
			m_dict.pop_back();
		m_dict.push_back(ItemInfo<KeyT>(key));
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & m_dict;
	}

	/*void dump ()
	{
		printf("dump:\n");
		for (size_t i = 0, ie = m_dict.size(); i < ie; ++i)
			printf("\titem: %3i %s\n", m_dict[i].m_use_count, m_dict[i].m_key.c_str());
	}*/

	size_t size () const { return m_dict.size(); }

	KeyT const & operator[] (size_t i) const { return m_dict[i]; }
};

