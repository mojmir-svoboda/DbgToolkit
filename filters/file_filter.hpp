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
#include <boost/tokenizer.hpp>
#include "nnode.hpp"

struct file_filter
{
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
	typedef NNode<std::string, bool> node_t;
	node_t * root;
	boost::char_separator<char> separator;

	file_filter ()
		: root(new node_t(std::string("/"), false))
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

	void append (std::string const & path)
	{
		if (!root)
			root = new node_t(std::string("/"), false);

		tokenizer_t tok(path, separator);
		node_t * level = root;
	   	tokenizer_t::const_iterator it = tok.begin(), ite = tok.end();
		while (it != ite)
		{
			node_t * n = node_t::node_child(level, *it);
			if (n == 0)
			{
				n = new node_t(*it, false);
				node_t::node_append(level, n);
			}
			level = n;
			++it;
			if (it == ite)
			{
				level->data = true;
			}
   		}
	}

	bool is_excluded (std::string const & file) const
	{
		tokenizer_t tok(file, separator);
		node_t * level = root;
	   	for (tokenizer_t::iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
		{
			level = node_t::node_child(level, *it);
			if (level == 0)
				return false; // node not in tree
			else if (level->data == true) // node is tagged for exclusion
				return true;  // node is tagged for exclusion
		}
		return false; // node is not tagged
	}

	void exclude_off (std::string const & file)
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
			if (it == ite && level->data == true)
			{
				level->data = false;
			}
		}
	}

	void reassemble_path (std::vector<node_t *> const & nodes, std::string & path) const
	{
		for (size_t i = 0, ie = nodes.size(); i < ie; ++i)
		{
			path.append(nodes[i]->key);
			if (i + 1 < ie)
				path.append("/");
		}
	}

	void export_filter_impl (node_t const * node, std::vector<node_t *> & nodes, std::string & output) const
	{
		if (node && node->children)
		{
			node_t * child = node->children;

			while (child)
			{
				node_t * current = child;
				nodes.push_back(current);
				if (current->data == true)
				{
					std::string path;
					path.reserve(128);
					reassemble_path(nodes, path);
					output.append("\n");
					output.append(path);
					path.clear();
				}
				export_filter_impl(current, nodes, output);
				nodes.pop_back();
				child = current->next;
			}
		}
	}

	void export_filter (std::string & output) const
	{
		output.reserve(128);
		std::vector<node_t *> path;
		path.reserve(32);
		export_filter_impl(root, path, output);
	}
};

