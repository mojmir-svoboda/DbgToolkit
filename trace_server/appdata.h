#pragma once
#include <vector>
#include <QString>
#include <cstdint>
#include <3rd/assocvector.h>

struct Dict : Loki::AssocVector<uint64_t, QString>
{
// 	QString findNameFor (QString const & strval) const
// 	{
// 		for (int i = 0, ie = m_dict.size(); i < ie; ++i)
// 			if (m_dict[i].first == strval)
// 				return m_dict[i].second;
// 		return QString();
// 	}

	void add (uint64_t val, QString const & name) { insert(std::make_pair(val, name)); }

	QString findNameFor (uint64_t val) const
	{
		auto it = find(val);
		if (it != end())
			return it->second;
		return QString();
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("dict", m_dict);
	}
};

struct AppData
{
	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("dict_lvl", m_dict_lvl);
		ar & boost::serialization::make_nvp("dict_ctx", m_dict_ctx);
	}

	Dict		m_dict_lvl;
	Dict		m_dict_ctx;
};

