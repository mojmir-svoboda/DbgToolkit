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
#include <string>
#include <cstdio>
#include <boost/tokenizer.hpp>
#include <boost/serialization/split_member.hpp>
#include "nnode.hpp"

template <class FilterT>
struct file_filter
{
	typedef FilterT filter_t;
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
	typedef NNode<std::string, filter_t> node_t;
	node_t * root;
	boost::char_separator<char> separator;

	file_filter ()
		: root(new node_t(std::string("/"), filter_t()))
		, separator(":/\\")
	{ }

	~file_filter ()
	{
		clear();
	}

	void clear ()
	{
		if (root)
		{
			node_t::node_destroy(root);
			root = 0;
		}
	}

	bool empty () const { return !(root && root->children); }

	void set_to_state (std::string const & file, filter_t const & data)
	{
		char const * const bgn = file.c_str();
		char const * const end = bgn + file.size() + 1;

		char const * left = bgn;
		char const * right = bgn;
		node_t * level = root;
		for (; right < end; ++right)
		{
			char const c = *right;
			if (c == ':' || c == '/' || c == '\\' || c == '\0' || right + 1 == end)
			{
				if (right - left > 0)
				{
					node_t * child = node_t::node_child_fast(level, left, right);
					if (child == 0)
					{
						child = new node_t(left, right, data);
						node_t::node_append(level, child);
					}
					level = child;
				}

				if (right + 1 < end)
					left = right + 1;
			}
		}

		level->data = data;
	}

	bool is_present (std::string const & file, filter_t & data) const
	{
		tokenizer_t tok(file, separator);
		node_t const * level = root;
	   	for (tokenizer_t::iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
		{
			level = node_t::node_child(level, *it);
			if (level == 0)
				return false; // node not in tree
			data = level->data;
		}
		return true;
	}

	bool is_present (std::string const & file, filter_t const * & fi) const
	{
		fi = 0;
		char const * const bgn = file.c_str();
		char const * const end = bgn + file.size() + 1;

		char const * left = bgn;
		char const * right = bgn;
		node_t const * level = root;
		for (; right < end; ++right)
		{
			char const c = *right;
			if (c == ':' || c == '/' || c == '\\' || c == '\0' || right + 1 == end)
			{
				if (right - left > 0)
				{
					level = node_t::node_child_fast(level, left, right);
					if (level == 0)
						return false; // node not in tree
					fi = &level->data;
				}
				if (right + 1 < end)
					left = right + 1;
			}
		}
		return true;
	}

	void set_state_to_childs (node_t * node, filter_t const & data)
	{
		node = node->children;
		while (node)
		{
			node->data = data;
			set_state_to_childs(node, data);
			node = node->next;
		}
	}

	void set_state_to_childs (std::string const & file, filter_t const & data)
	{
		tokenizer_t tok(file, separator);
		node_t * level = root;
	   	tokenizer_t::const_iterator it = tok.begin(), ite = tok.end();
		while (it != ite)
		{
			level = node_t::node_child(level, *it);
			if (level == 0)
				return;

			++it;
			if (it == ite)
			{
				level->data = data;
				set_state_to_childs(level, data);
			}
		}
	}

	void set_state_to_parents (node_t * node, filter_t const & data)
	{
		while (node = node->parent)
		{
			node->data = data;
			set_state_to_parents(node, data);
		}
	}
/*
	void set_state_to_topdown (std::string const & file, E_NodeStates fw_state, E_NodeStates rev_state)
	{
		tokenizer_t tok(file, separator);
		node_t * level = root;
	   	tokenizer_t::const_iterator it = tok.begin(), ite = tok.end();
		while (it != ite)
		{
			level = node_t::node_child(level, *it);
			if (level == 0)
				return;

			++it;
			if (it == ite)
			{
				level->data.m_state = fw_state;
				set_state_to_childs(level, fw_state);
				set_state_to_parents(level, rev_state);
			}
		}
	}
*/
	void reassemble_path (std::vector<node_t *> const & nodes, std::string & path) const
	{
		for (size_t i = 0, ie = nodes.size(); i < ie; ++i)
		{
			path.append(nodes[i]->key);
			if (i + 1 < ie)
				path.append("/");
		}
	}

	template <class ArchiveT>
	inline void save (ArchiveT & a, unsigned const /*version*/) const
	{
		a.register_type(static_cast<node_t *>(NULL));
		a & root;
	}
	 
	template <class ArchiveT>
	inline void load (ArchiveT & a, unsigned const /*version*/)
	{
		a.register_type(static_cast<node_t *>(NULL));
		a & root;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()
	 
	void dump_filter_impl (node_t const * node, std::vector<node_t *> & nodes, std::string & output) const
	{
		if (node && node->children)
		{
			node_t * child = node->children;

			while (child)
			{
				node_t * current = child;
				nodes.push_back(current);
				{

					std::string path;
					path.reserve(128);
					reassemble_path(nodes, path);
					output.append("\n");

					if (current->data.m_state == e_Checked)
						output += "X ";
					else if (current->data.m_state == e_PartialCheck)
						output += "# ";
					else
						output += "- ";

					if (current->data.m_collapsed)
						output += "c ";
					else
						output += "E ";
					output.append(path);
					path.clear();
				}
				dump_filter_impl(current, nodes, output);
				nodes.pop_back();
				child = current->next;
			}
		}
	}

	void dump_filter (std::string & output) const
	{
		output.reserve(128);
		std::vector<node_t *> path;
		path.reserve(32);
		dump_filter_impl(root, path, output);
	}
};

