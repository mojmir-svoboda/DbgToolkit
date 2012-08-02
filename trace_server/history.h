#pragma once
#include <map>
#include <memory>

struct ItemInfo
{
	int m_use_count;

	ItemInfo () : m_use_count(0) { }
};

template <typename KeyT>
struct History
{
	unsigned m_item_limit;
	typedef std::map<KeyT, ItemInfo> dict_t;
	dict_t m_dict;

	History ()
		: m_item_limit(16)
	{ }

	void insert (KeyT const & k)
	{
		size_t n = m_dict.size();
		dict_t::iterator it = m_dict.find(k);
		if (it == m_dict.end())
		{
			if (n > m_item_limit)
			{
				evict();
			}
			it = m_dict.insert(std::make_pair(k, ItemInfo()));
		}
		++(it->second.m_use_count);
	}
};
