#pragma once
#include <QString>
#include <string>
#include <boost/serialization/string.hpp>
 
namespace boost { namespace serialization {
 
	template <class ArchiveT>
	inline void save (ArchiveT & a, QString const & q, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		std::string s = q.toStdString();
		a << make_nvp("value", s);
	}
	 
	template <class ArchiveT>
	inline void load (ArchiveT & a, QString & q, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		std::string s;
		a >> make_nvp("value", s);
		q = QString::fromStdString(s);
	}
	 
	template <class ArchiveT>
	inline void serialize (ArchiveT & a, QString & q, unsigned const version)
	{
		boost::serialization::split_free(a, q, version);
	}
	 
} }

