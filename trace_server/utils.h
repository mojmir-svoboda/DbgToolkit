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
#include <QString>
#include <QDir>
#include <QFile>
#include <QSettings>
#include "sessionstate.h"
#include "constants.h"

template<class C>
void clearListView (C * v)
{
	if (v && v->model())
	{
		static_cast<QStandardItemModel *>(v->model())->clear();
	}
}

inline void write_list_of_strings (QSettings & settings, char const * groupname, char const * groupvaluename, QList<QString> const & lst)
{
	settings.beginWriteArray(groupname);
	for (int i = 0, ie = lst.size(); i < ie; ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue(groupvaluename, lst.at(i));
		qDebug("store to registry %i/%i: %s", i,ie, lst.at(i).toStdString().c_str());
	}
	settings.endArray();
}

inline void read_list_of_strings (QSettings & settings, char const * groupname, char const * groupvaluename, QList<QString> & lst)
{
	int const size = settings.beginReadArray(groupname);
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex(i);
		QString val = settings.value(groupvaluename).toString();
		//qDebug("read from registry: %s", val.toStdString().c_str());
		lst.push_back(val);
	}
	settings.endArray();
}


inline QString getPresetFileName (QString const & appdir, QString const & preset_name)
{
	QString presetdir = appdir + "/" + preset_name;
	QDir d;
	d.mkpath(presetdir);
	QString const fname = presetdir + "/" + g_presetFileName;
	return fname;
}

inline QString createPresetPath (QString const & appdir, QString const & preset_name)
{
	QString presetdir = appdir + "/" + preset_name;
	QDir d;
	d.mkpath(presetdir);
	return presetdir;
}


inline QString getDataTagFileName (QString const & appdir, QString const & app_name, QString const & preset_name, QString const & class_name, QString const & tag)
{
	QString presetdir = appdir + "/" + app_name + "/" + preset_name + "/" + class_name;
	QDir d;
	d.mkpath(presetdir);
	QString const fname = presetdir + "/" + tag;
	return fname;
}

inline QString getDataTagFileName (QString const & appdir, QString const & preset_name, QString const & class_name, QString const & tag)
{
	QString presetdir = appdir + "/" + preset_name + "/" + class_name;
	QDir d;
	d.mkpath(presetdir);
	QString const fname = presetdir + "/" + tag;
	return fname;
}



inline bool existsFile (char const * fname)
{
	QFile file(fname);
	return file.exists();
}

inline QString getPresetPath (QString const & app_name, QString const & name)
{
	QString path;
	path.append(app_name);
	path.append("/");
	path.append(name);
	return path;
}

inline void reassemblePath (std::vector<QString> const & tokens, QString & path)
{
	for (std::vector<QString>::const_reverse_iterator it = tokens.rbegin(), ite = tokens.rend(); it != ite; ++it)
	{
		path += *it;
		path += '/';
	}
}

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

