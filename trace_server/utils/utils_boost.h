#pragma once
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>

template <typename H, typename T>
struct do_recurse
{
	template <typename FnT>
	static void apply (boost::tuples::cons<H,T> const & c, FnT fn)
	{
		typedef boost::tuples::cons<H,T> current_cons;
		typedef typename current_cons::tail_type tail_cons;
		typedef typename tail_cons::head_type tail_head;
		typedef typename tail_cons::tail_type tail_tail;

		fn(c.get_head());
		do_recurse<tail_head, tail_tail>::apply(c.get_tail(), fn);
	}

	template <typename FnT>
	static void apply (boost::tuples::cons<H,T> & c, FnT fn)
	{
		typedef boost::tuples::cons<H,T> current_cons;
		typedef typename current_cons::tail_type tail_cons;
		typedef typename tail_cons::head_type tail_head;
		typedef typename tail_cons::tail_type tail_tail;

		fn(c.get_head());
		do_recurse<tail_head, tail_tail>::apply(c.get_tail(), fn);
	}
};

template <typename H>
struct do_recurse<H, boost::tuples::null_type>
{
	template <typename FnT>
	static void apply (boost::tuples::cons<H,boost::tuples::null_type> const & c, FnT fn) { }
	template <typename FnT>
	static void apply (boost::tuples::cons<H,boost::tuples::null_type> & c, FnT fn) { }
};

template <typename H, typename T, typename FnT>
void recurse (boost::tuples::cons<H,T> const & c, FnT fn)
{
	do_recurse<H,T>::apply(c, fn);
}

template <typename H, typename T, typename FnT>
void recurse (boost::tuples::cons<H,T> & c, FnT fn)
{
	do_recurse<H,T>::apply(c, fn);
}

