#pragma once
#include <QByteArray>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/collection_size_type.hpp>
#include <boost/serialization/binary_object.hpp>

namespace boost { namespace serialization {

	template <class ArchiveT>
	inline void save (ArchiveT & a, QByteArray const & arr, const unsigned)
	{
		collection_size_type count(arr.size());
		a << BOOST_SERIALIZATION_NVP(count);
		void * data = const_cast<void *>(static_cast<void const *>(arr.data()));
		a & make_nvp("blob", make_binary_object(data, count));
	}

	template <class ArchiveT>
	inline void load (ArchiveT & a, QByteArray & arr, const unsigned)
	{
		collection_size_type count(0);
		a >> BOOST_SERIALIZATION_NVP(count);

		QByteArray arr2(count, Qt::Uninitialized);
		a & make_nvp("blob", make_binary_object(arr2.data(), count));

		arr = std::move(arr2);
	}

	template <class ArchiveT>
	inline void serialize (ArchiveT & a, QByteArray & arr, unsigned const version)
	{
		boost::serialization::split_free(a, arr, version);
	}

} }


