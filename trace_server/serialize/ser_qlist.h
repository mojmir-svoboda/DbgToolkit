#pragma once

namespace boost { namespace serialization {

	template <class ArchiveT, class T>
	inline void save (ArchiveT & a, QList<T> const & l, const unsigned)
	{
		boost::serialization::stl::save_collection<ArchiveT, QList<T> >(a, l);
	}

	template <class ArchiveT, class T>
	inline void load (ArchiveT & a, QList<T> & l, const unsigned)
	{
		using namespace boost::serialization::stl;
		load_collection<ArchiveT, QList<T>, archive_input_seq<ArchiveT, QList<T> >, no_reserve_imp< QList<T> > >(a, l);
	}

	template <class ArchiveT, class T>
	inline void serialize (ArchiveT & a, QList<T> & l, unsigned const version)
	{
		boost::serialization::split_free(a, l, version);
	}

} }

BOOST_SERIALIZATION_COLLECTION_TRAITS(QList)

