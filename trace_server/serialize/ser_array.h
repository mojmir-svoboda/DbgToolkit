namespace boost {
namespace serialization {

template<class Archive, class T, size_t N>
void serialize(Archive & ar, std::array<T,N> & a, const unsigned int version)
{
  ar & boost::serialization::make_array(a.data(), a.size());
}

} // namespace serialization
} // namespace boost