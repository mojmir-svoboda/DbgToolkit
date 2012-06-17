#include <QRegExp>
#include <string>
#include <boost/serialization/string.hpp>
 
namespace boost { namespace serialization {
 
	template <class ArchiveT>
	inline void save (ArchiveT & a, QRegExp const & q, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		a << make_nvp("value", q.pattern().toStdString());
	}
	 
	template <class ArchiveT>
	inline void load (ArchiveT & a, QRegExp & q, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		std::string s;
		a >> make_nvp("value", s);
		q.setCaseSensitivity(Qt::CaseSensitive);
		q.setPatternSyntax(QRegExp::RegExp);
		q.setPattern(QString::fromStdString(s));
	}
	 
	template <class ArchiveT>
	inline void serialize (ArchiveT & a, QRegExp & q, unsigned const version)
	{
		boost::serialization::split_free(a, q, version);
	}
	 
} }

