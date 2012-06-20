#pragma once
#include <string>
#include <vector>

struct ThreadSpecific
{
	std::vector<std::string> m_tids;
	std::vector<unsigned> m_indents;

	ThreadSpecific ()
	{
		m_tids.reserve(16);
		m_indents.reserve(16);
	}

	int findThreadId (std::string const & str)
	{
		for (size_t i = 0, ie = m_tids.size(); i < ie; ++i)
			if (m_tids[i] == str)
				return i;
		return registerThread(str);
	}
	int registerThread (std::string const & str)
	{
		m_tids.push_back(str);
		m_indents.push_back(0);
		return m_tids.size() - 1;
	}

	int incrIndent (int idx)
	{
		if (idx >= 0 && idx < (int)m_indents.size())
			return ++m_indents[idx];
		return 0;
	}
	int decrIndent (int idx)
	{
		if (idx >= 0 && idx < (int)m_indents.size())
			return --m_indents[idx];
		return 0;
	}

	void clear ()
	{
		m_tids.clear();
		m_indents.clear();
	}
};


