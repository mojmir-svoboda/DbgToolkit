#pragma once
#include <boost/call_traits.hpp>

template <class Key, class Data>
struct NNode
{
	typedef typename boost::call_traits<Key>::param_type key_param_t;
	Key 	key;
	Data	data;
	NNode * next;
	NNode * prev;
	NNode * parent;
	NNode * children;

	NNode () : key(), data(), next(0), prev(0), parent(0), children(0) { }
	explicit NNode (key_param_t k, typename boost::call_traits<Data>::param_type d)
		: key(k), data(d), next(0), prev(0), parent(0), children(0)
	{ }

	~NNode () { }

	static NNode * node_append (NNode * parent, NNode * node) { return node_insert_before(parent, NULL, node); }
	static NNode * node_prepend (NNode * parent, NNode * node) { return node_insert_before (parent, parent->children, node); }
	static NNode * node_insert_before (NNode * parent, NNode * sibling, NNode * node)
	{
		node->parent = parent;

		if (sibling)
		{
			if (sibling->prev)
			{
				node->prev = sibling->prev;
				node->prev->next = node;
				node->next = sibling;
				sibling->prev = node;
			}
			else
			{
				node->parent->children = node;
				node->next = sibling;
				sibling->prev = node;
			}
		}
		else
		{
			if (parent->children)
			{
				sibling = parent->children;
				while (sibling->next)
					sibling = sibling->next;
				node->prev = sibling;
				sibling->next = node;
			}
			else
				node->parent->children = node;
		}
		return node;
	}

	static NNode * node_child (NNode * node, key_param_t k)
	{     
		node = node->children;
		while (node)
		{
			if (node->key == k)
				return node;
			node = node->next;
		}
		return 0;
	}

	static bool is_leaf (NNode * node) { return node->children == 0; }
	static bool is_root (NNode * node) { return node->parent == 0 && node->prev == 0 && node->next == 0; }

	static void node_destroy (NNode * root)
	{
		if (!is_root(root))
			node_unlink(root);
		nodes_free(root);
	}

	static void node_unlink (NNode * node)
	{ 
		if (node->prev)
			node->prev->next = node->next;
		else if (node->parent)
			node->parent->children = node->next;
		node->parent = NULL;
		if (node->next)
		{
			node->next->prev = node->prev;
			node->next = NULL;
		}
		node->prev = NULL;
	}

	static void nodes_free (NNode * node)
	{
		while (node)
		{     
			NNode * next = node->next;
			if (node->children)
				nodes_free(node->children);
			delete node;
			node = next;
		}
	}
};
