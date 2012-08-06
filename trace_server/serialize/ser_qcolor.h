#include <QString>
#include <QColor>
#include <string>
#include <boost/serialization/string.hpp>
 
namespace boost { namespace serialization {
 
	template <class ArchiveT>
	inline void save (ArchiveT & a, QColor const & q, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		std::string s = q.name().toStdString();
		a << make_nvp("value", s);
	}
	 
	template <class ArchiveT>
	inline void load (ArchiveT & a, QColor & q, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		std::string s;
		a >> make_nvp("value", s);
		q.setNamedColor(QString::fromStdString(s));
	}
	 
	template <class ArchiveT>
	inline void serialize (ArchiveT & a, QColor & q, unsigned const version)
	{
		boost::serialization::split_free(a, q, version);
	}
	 
} }

