#pragma once
#include "ser_qlist.h"
#include "ser_qstring.h"

namespace boost { namespace serialization {

	template <class ArchiveT>
	inline void save (ArchiveT & a, QStringList const & sl, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		QList<QString> const & ref = sl;
		a << make_nvp("value", ref);
	}
	 
	template <class ArchiveT>
	inline void load (ArchiveT & a, QStringList & sl, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		QList<QString> & ref = sl;
		a >> make_nvp("value", ref);
	}
	 
	template <class ArchiveT>
	inline void serialize (ArchiveT & a, QStringList & sl, unsigned const version)
	{
		boost::serialization::split_free(a, sl, version);
	}
	
} }


