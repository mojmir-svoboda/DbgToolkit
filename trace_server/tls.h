#pragma once
#include <string>
#include <vector>

struct ThreadSpecific
{
	std::vector<unsigned long long> m_times;
	std::vector<QString> m_tids;
	std::vector<unsigned> m_indents;

	ThreadSpecific ()
	{
		m_tids.reserve(16);
		m_indents.reserve(16);
		m_times.reserve(16);
	}

	int findThreadId (QString const & str)
	{
		for (size_t i = 0, ie = m_tids.size(); i < ie; ++i)
			if (m_tids[i] == str)
				return static_cast<int>(i);
		return registerThread(str);
	}
	int registerThread (QString const & str)
	{
		m_tids.push_back(str);
		m_indents.push_back(0);
		m_times.push_back(0);
		return static_cast<int>(m_tids.size()) - 1;
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

	void setLastTime (int idx, unsigned long long t)
	{
		if (idx >= 0 && idx < (int)m_times.size())
			m_times[idx] = t;
	}

	unsigned long long lastTime (int idx)
	{
		if (idx >= 0 && idx < (int)m_times.size())
			return --m_times[idx];
		return 0;
	}

	void clear ()
	{
		m_tids.clear();
		m_indents.clear();
	}
};


