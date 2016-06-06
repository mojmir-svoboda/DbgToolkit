#pragma once

template <typename T> struct hash;

template <std::size_t = sizeof(std::size_t)>
struct FnvHash
{
	static std::size_t hash (char const * __first, std::size_t __length)
	{
		std::size_t __result = 0;
		for (; __length > 0; --__length)
			__result = (__result * 131) + *__first++;
		return __result;
	}
};

template <>
struct FnvHash<4>
{
	static std::size_t hash (char const * __first, std::size_t __length)
	{
		std::size_t __result = static_cast<std::size_t>(2166136261UL);
		for (; __length > 0; --__length)
		{
			__result ^= static_cast<std::size_t>(*__first++);
			__result *= static_cast<std::size_t>(16777619UL);
		}
		return __result;
	}
};

template<>
struct FnvHash<8>
{
	static std::size_t hash (char const * __first, std::size_t __length)
	{
		unsigned long long __result = static_cast<unsigned long long>(14695981039346656037ULL);
		for (; __length > 0; --__length)
		{
			__result ^= static_cast<unsigned long long>(*__first++);
			__result *= static_cast<unsigned long long>(1099511628211ULL);
		}
		return __result;
	}
};

template <>
struct hash<std::string>
	: public std::unary_function<std::string, std::size_t>
{      
	std::size_t operator() (const std::string& __s) const
	{
		return FnvHash<>::hash(__s.data(), __s.length());
	}
};

template <>
struct hash<QString>
	: public std::unary_function<QString, std::size_t>
{      
	std::size_t operator() (QString const & __s) const
	{
		return FnvHash<>::hash(__s.toLatin1(), __s.length());
	}
};
