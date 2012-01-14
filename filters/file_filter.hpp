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
		node_t::node_destroy(root);
		root = 0;
	}

	void append (std::string const & path)
	{
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
				//qDebug("\t tag:    node_ptr=%08x (key=\'%s\')\n", level, level->key.c_str());
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
				//qDebug("\t unTAG!:    node_ptr=%08x (key=\'%s\')\n", level, level->key.c_str());
				level->data = false;
			}
		}
	}
};

