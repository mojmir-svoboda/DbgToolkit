#pragma once
#include <QVector>

namespace boost { namespace serialization {

	template <class ArchiveT, class T>
	inline void save (ArchiveT & a, QVector<T> const & l, const unsigned)
	{
		boost::serialization::stl::save_collection<ArchiveT, QVector<T> >(a, l);
	}

	template <class ArchiveT, class T>
	inline void load (ArchiveT & a, QVector<T> & l, const unsigned)
	{
		using namespace boost::serialization::stl;
		load_collection<ArchiveT, QVector<T>, archive_input_seq<ArchiveT, QVector<T> >, reserve_imp< QVector<T> > >(a, l);
	}

	template <class ArchiveT, class T>
	inline void serialize (ArchiveT & a, QVector<T> & l, unsigned const version)
	{
		boost::serialization::split_free(a, l, version);
	}

} }

//BOOST_SERIALIZATION_COLLECTION_TRAITS(QVector)

