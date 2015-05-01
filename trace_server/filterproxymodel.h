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
#include <table/baseproxymodel.h>
#include "basetablemodel.h"

namespace logs { class LogTableView; struct LogConfig; }
struct FilterMgr;

class FilterProxyModel : public BaseProxyModel
{
	Q_OBJECT
public:
	explicit FilterProxyModel (QObject * parent, FilterMgr const * f, BaseTableModel * m);
	virtual ~FilterProxyModel ();

	virtual QModelIndex sibling (int row, int column, QModelIndex const & idx) const override;
	virtual bool filterAcceptsRow (int sourceRow, QModelIndex const & sourceParent) const override;
	virtual bool filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const override;
	void setSourceModel (QAbstractItemModel *sourceModel);
	virtual Qt::ItemFlags flags (QModelIndex const & index) const;
	void resizeToCfg (logs::LogConfig const & config);
	void commitBatchToModel (int from, int to, BatchCmd const & batch);

	virtual void clearModel () override;
	virtual void clearModelData () override;

protected:
	BaseTableModel * m_dcmds_model;
	int m_column_count;
	FilterMgr const * m_filter_mgr;
};

// extended filter with scopes and maybe more
struct ExtFilterProxyModel : FilterProxyModel
{
	bool m_scopes_enabled;
	bool m_dt_scopes_enabled;
	
	explicit ExtFilterProxyModel (QObject * parent, FilterMgr const * f, BaseTableModel * m);
	virtual bool filterAcceptsRow (int sourceRow, QModelIndex const & sourceParent) const override;
};
