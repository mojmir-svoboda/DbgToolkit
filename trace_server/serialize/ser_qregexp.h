#pragma once
#include <QRegularExpression>
#include <string>
#include <boost/serialization/string.hpp>
 
namespace boost { namespace serialization {
 
	template <class ArchiveT>
	inline void save (ArchiveT & a, QRegularExpression const & q, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		std::string s = q.pattern().toStdString();
		a << make_nvp("value", s);
	}
	 
	template <class ArchiveT>
	inline void load (ArchiveT & a, QRegularExpression & q, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		std::string s;
		a >> make_nvp("value", s);
		//q.setCaseSensitivity(Qt::CaseSensitive);
		//q.setPatternSyntax(QRegularExpression::RegExp);
		q.setPattern(QString::fromStdString(s));
	}
	 
	template <class ArchiveT>
	inline void serialize (ArchiveT & a, QRegularExpression & q, unsigned const version)
	{
		boost::serialization::split_free(a, q, version);
	}
	 
} }

