/**
 * Copyright (C) 2011 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once
#include <filters/file_filter.hpp>

template <class F>
void set_state_to_childs (tree_filter<F> const & ff, std::string const & file, E_NodeStates state);
template <class F>
void set_state_to_parents (tree_filter<F> const & ff, typename tree_filter<F>::node_t * node, E_NodeStates state);
template <class F>
void set_state_to_childs (tree_filter<F> const & ff, typename tree_filter<F>::node_t * node, E_NodeStates state);

template <class F>
inline void set_state_to_topdown (tree_filter<F> const & ff, std::string const & file, E_NodeStates fw_state, E_NodeStates rev_state)
{
	typedef typename tree_filter<F>::tokenizer_t tokenizer_t;
	tokenizer_t tok(file, ff.separator);
	typename tree_filter<F>::node_t * level = ff.root;
	typename tokenizer_t::const_iterator it = tok.begin(), ite = tok.end();
	while (it != ite)
	{
		level = tree_filter<F>::node_t::node_child(level, *it);
		if (level == 0)
			return;

		++it;
		if (it == ite)
		{
			level->data.m_state = fw_state;
			set_state_to_childs(ff, level, fw_state);
			set_state_to_parents(ff, level, rev_state);
		}
	}
}

template <class F>
inline void set_state_to_childs (tree_filter<F> const & ff, std::string const & file, E_NodeStates state)
{
	typename tree_filter<F>::tokenizer_t tok(file, ff.separator);
	typename tree_filter<F>::node_t * level = ff.root;
	typename tree_filter<F>::tokenizer_t::const_iterator it = tok.begin(), ite = tok.end();
	while (it != ite)
	{
		level = tree_filter<F>::node_t::node_child(level, *it);
		if (level == 0)
			return;

		++it;
		if (it == ite)
		{
			level->data.m_state = state;
			set_state_to_childs(ff, level, state);
		}
	}
}

template <class F>
inline void set_state_to_childs (tree_filter<F> const & ff, typename tree_filter<F>::node_t * node, E_NodeStates state)
{
	node = node->children;
	while (node)
	{
		node->data.m_state = state;
		set_state_to_childs(ff, node, state);
		node = node->next;
	}
}

template <class F>
inline void set_state_to_parents (tree_filter<F> const & ff, typename tree_filter<F>::node_t * node, E_NodeStates state)
{
	while (node = node->parent)
	{
		node->data.m_state = state;
		set_state_to_parents(ff, node, state);
	}
}

