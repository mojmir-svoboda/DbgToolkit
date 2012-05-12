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
	QStandardItem * name_item = new QStandardItem(str);
	name_item->setCheckable(true);
	name_item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	row_items << name_item;
	return row_items;
}

inline QList<QStandardItem *> addRowTriState (QString const & str, bool checked, E_FilterMode filt_mode)
{
	QList<QStandardItem *> row_items;
	QStandardItem * name_item = new QStandardItem(str);
	name_item->setCheckable(true);
	name_item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	name_item->setTristate(true);
	row_items << name_item;
	return row_items;
}

inline QList<QStandardItem *> addRow (QString const & str, bool checked, E_FilterMode filt_mode)
{
	QList<QStandardItem *> row_items;
	QStandardItem * name_item = new QStandardItem(str);
	name_item->setCheckable(true);
	name_item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);

	QString mode_str = filt_mode == e_Include ? QString("Include") : QString("Exclude");
	QStandardItem * flt_item = new QStandardItem(mode_str);
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
		QStandardItem * child = node->child(r, 0);
		flipCheckStateRecursive(child);
	}
}

inline void setCheckState (QStandardItem * item, bool checked)
{
	item->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
}

inline void setCheckStateRecursive (QStandardItem * node, bool checked)
{
	setCheckState(node, checked);
	int const rc = node->rowCount();
	for (int r = 0; r < rc; ++r)
	{
		QStandardItem * child = node->child(r, 0);
		setCheckStateRecursive(child, checked);
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


