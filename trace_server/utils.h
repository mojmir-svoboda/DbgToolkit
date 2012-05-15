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
#include <QStandardItem>
#include <QString>
#include "sessionstate.h"

inline QStandardItem * findChildByText (QStandardItem * parent, QString const & txt)
{
	for (size_t i = 0, ie = parent->rowCount(); i < ie; ++i)
	{
		if (parent->child(i)->text() == txt)
			return parent->child(i);
	}
	return 0;
}

inline QList<QStandardItem *> addRow (QString const & str, bool checked )
{
	QList<QStandardItem *> row_items;
	QStandardItem * const name_item = new QStandardItem(str);
	name_item->setCheckable(true);
	name_item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	row_items << name_item;
	return row_items;
}

inline QList<QStandardItem *> addRowTriState (QString const & str, bool checked, E_FilterMode filt_mode)
{
	QList<QStandardItem *> row_items;
	QStandardItem * const name_item = new QStandardItem(str);
	name_item->setCheckable(true);
	name_item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	name_item->setTristate(true);
	row_items << name_item;
	return row_items;
}

inline void flipCheckState (QStandardItem * item)
{
	bool const checked = (item->checkState() == Qt::Checked);
	item->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
}

inline void flipCheckStateRecursive (QStandardItem * node)
{
	flipCheckState(node);
	int const rc = node->rowCount();
	for (int r = 0; r < rc; ++r)
	{
		QStandardItem * const child = node->child(r, 0);
		flipCheckStateRecursive(child);
	}
}

inline void setCheckState (QStandardItem * const node, Qt::CheckState const state)
{
	node->setCheckState(state);
}

inline void setCheckStateRecursive (QStandardItem * const node, Qt::CheckState const state)
{
	setCheckState(node, state);
	int const rc = node->rowCount();
	//qDebug("node=%s, state=%u", node->text().toStdString().c_str(), state);
	for (int r = 0; r < rc; ++r)
	{
		QStandardItem * const child = node->child(r, 0);
		setCheckStateRecursive(child, state);
	}
}

inline void setCheckStateReverse (QStandardItem * n, Qt::CheckState state)
{
	while (n)
	{
		n->setCheckState(state);
		n = n->parent();
	}
}

inline void collectPath (QStandardItem * n, std::vector<QString> & tokens)
{
	while (n)
	{
		tokens.push_back(n->text());
		n = n->parent();
	}
}

inline void reassemblePath (std::vector<QString> const & tokens, QString & path)
{
	for (std::vector<QString>::const_reverse_iterator it = tokens.rbegin(), ite = tokens.rend(); it != ite; ++it)
	{
		path += *it;
		path += '/';
	}
}

inline void syncCheckBoxesWithFileFilters (QStandardItem const * node, E_FilterMode const fmode, file_filter & ff, strings_t & stack)
{
	stack.push_back(node->text().toStdString());
	bool const checked = (node->checkState() == Qt::Checked);
	if (fmode == e_Include)
		ff.exclude_to_state(stack, !checked);
	else
		ff.exclude_to_state(stack, checked);

	bool const descend = ((fmode == e_Include && checked) || (fmode == e_Exclude && !checked));
	if (descend)
	{
		int const rc = node->rowCount();
		for (int r = 0; r < rc; ++r)
		{
			QStandardItem const * const child = node->child(r, 0);
			syncCheckBoxesWithFileFilters(child, fmode, ff, stack);
		}
	}
	stack.pop_back();
}

inline void syncCheckBoxesWithFileFilters (QStandardItem const * node, E_FilterMode const fmode, file_filter & ff)
{
	strings_t stack;
	stack.reserve(256);
	syncCheckBoxesWithFileFilters(node, fmode, ff, stack);
}


