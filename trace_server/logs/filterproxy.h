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
#include <QAbstractProxyModel>
#include <QRegExp>
#include <filters/file_filter.hpp>
#include "../filterstate.h"

class MainWindow;

namespace logs { class LogWidget; }

class FilterProxyModel : public QAbstractProxyModel
{
	Q_OBJECT

public:
	explicit FilterProxyModel (QObject * parent, logs::LogWidget & lw);

	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent (QModelIndex const & child) const;

	virtual int rowCount (QModelIndex const & parent = QModelIndex()) const;
	virtual int columnCount (QModelIndex const & parent = QModelIndex()) const;

	virtual QModelIndex mapToSource (QModelIndex const & proxyIndex) const;
	virtual QModelIndex mapFromSource (QModelIndex const & sourceIndex) const;

	virtual bool insertRows (int row, int count, QModelIndex const &);
	virtual bool insertColumns (int column, int count, const QModelIndex &parent = QModelIndex());
	QVariant data (QModelIndex const & index, int role) const;
	Qt::ItemFlags flags (QModelIndex const & index) const;

public slots:
	void force_update();

protected:

	bool filterAcceptsRow (int sourceRow, QModelIndex const & sourceParent) const;

	std::vector<int> m_map_from_tgt;
	logs::LogWidget & m_log_widget;
	FilterState & m_filter_state;
	int m_columns;
	std::vector<int> m_map_from_src;
	MainWindow const * m_main_window;
};


