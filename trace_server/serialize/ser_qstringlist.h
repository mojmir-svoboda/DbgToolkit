#pragma once

namespace boost { namespace serialization {

	template <class ArchiveT>
	inline void save (ArchiveT & a, QStringList const & sl, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		a << make_nvp("value", sl);
	}
	 
	template <class ArchiveT>
	inline void load (ArchiveT & a, QStringList & sl, unsigned const /*version*/)
	{
		using boost::serialization::make_nvp;
		std::string s;
		a >> make_nvp("value", sl);
	}
	 
	template <class ArchiveT>
	inline void serialize (ArchiveT & a, QStringList & sl, unsigned const version)
	{
		boost::serialization::split_free(a, sl, version);
	}
	
} }

