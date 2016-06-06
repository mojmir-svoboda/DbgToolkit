#pragma once
#include <cstring>
#include <cassert>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <tmpl/tmpl.h>
#include <utils_qt/qstringdatatable.h>
#include <utils_qt/utils_tuple.h>

using qtcolor = uint32_t;

namespace logs { namespace proto {

	enum tags : unsigned {
		tag_stime,
		tag_ctime,
		tag_dt,
		tag_lvl,
		tag_ctx,
		tag_tid,
		tag_file,
		tag_line,
		tag_func,
		tag_msg,
		e_max_tag_count
	};
	char const * const tag_names[] = {
		"stm",
		"ctm",
		"dt",
		"lvl",
		"ctx",
		"tid",
		"file",
		"line",
		"func",
		"msg",
		"N/A"
	};
	inline constexpr size_t get_tag_count () { return e_max_tag_count; }

	inline char const * get_tag_name (size_t i)
	{
		if (i < e_max_tag_count)
			return tag_names[i];
		return 0;
	}

	inline tags tag_for_name (char const * tag_name)
	{
		for (size_t i = 0; i < e_max_tag_count; ++i)
			if (strcmp(tag_names[i], tag_name) == 0)
				return static_cast<tags>(i);
		assert(0);
		return e_max_tag_count;
	}

	template <class... T> struct typelist { };

	template<unsigned N>
	using int_ = std::integral_constant<unsigned, N>;

	using protocol_definition = typelist<
			typelist<int_<tag_stime>, int_<0>, uint64_t>
		, typelist<int_<tag_ctime>, int_<1>, uint64_t>
		, typelist<int_<tag_dt>		, int_<2>, uint64_t>
		, typelist<int_<tag_lvl>	, int_<3>, uint32_t>
		, typelist<int_<tag_ctx>	, int_<4>, uint64_t>
		, typelist<int_<tag_tid>	, int_<5>, uint64_t>
		, typelist<int_<tag_file> , int_<6>, stroffs_t>
		, typelist<int_<tag_line> , int_<7>, uint32_t>
		, typelist<int_<tag_func> , int_<8>, stroffs_t>
		, typelist<int_<tag_msg>	, int_<9>, stroffs_t>
		>;

	using log_format = typelist<
			int_<tag_stime>
		, int_<tag_dt>
		, int_<tag_ctime>
		, int_<tag_tid>
		, int_<tag_lvl>
		, int_<tag_ctx>
		, int_<tag_file>
		, int_<tag_line>
		, int_<tag_func>
		, int_<tag_msg>
		>;

	template<class L>
	struct tmpl_sizeof_impl;
	template<template<class...> class L, class... T>
	struct tmpl_sizeof_impl<L<T...>>
	{
		using type = std::integral_constant<std::size_t, sizeof...(T)>;
	};
	template<class L>
	using tmpl_size = typename tmpl_sizeof_impl<L>::type;

	template<class L, class V>
	struct tmpl_fill_impl;
	template<template<class...> class L, class... T, class V>
	struct tmpl_fill_impl<L<T...>, V>
	{
		template<typename T> struct MSVC_Bug_Workaround
		{
			typedef V type;
		};
		using type = L<typename MSVC_Bug_Workaround<T>::type...>;
	};
	template<class L, class V>
	using tmpl_fill = typename tmpl_fill_impl<L, V>::type;


	template<class Tag>
	using tag2type = tmpl_at_c<tmpl_map_find<protocol_definition, Tag>, 2>;
	using tag_tid_type = tag2type<int_<tag_tid>>;
	using values_t = tmpl_rename<tmpl_transform<tag2type, log_format>, std::tuple>;

	//print<get_colors_t<log_format>> p;
	using fgcols_t = tmpl_rename<tmpl_fill<log_format, qtcolor>, std::tuple>;
	using bgcols_t = tmpl_rename<tmpl_fill<log_format, qtcolor>, std::tuple>;
	using indents_t = tmpl_rename<tmpl_fill<log_format, uint32_t>, std::tuple>;

	struct Flags
	{
		unsigned m_scope_type : 2;
		unsigned pad : 30;

		Flags ()  : m_scope_type(0), pad(0) { }
	};
	using flags_t = tmpl_rename<tmpl_fill<log_format, Flags>, std::tuple>;

	template<class Tag>
	using tag2col = tmpl_indexof<log_format, Tag>;

	template<size_t Col>
	using col2tag = tmpl_at_c<log_format, Col>;

	template<int N>
	constexpr tags getNthTag () { return static_cast<tags>(col2tag<N>::value); }
	template<int... Ns>
	tags getTagForCol (int index, seq<Ns...>)
	{
		using get_fn_prototype = tags(*)();
		constexpr static get_fn_prototype const funcs[] = { &getNthTag<Ns>... };
		get_fn_prototype const & nth = funcs[index];
		return (*nth)();
	}
	inline tags getTagForCol (int index) { return getTagForCol(index, gen_seq<std::tuple_size<values_t>::value>{ }); }

}}

Q_DECLARE_METATYPE(logs::proto::Flags)
