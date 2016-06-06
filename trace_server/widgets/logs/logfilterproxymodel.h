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
#include <models/filterproxymodel.h>

namespace logs {

	struct FilterMgr;
	struct LogTableModel;

	class FilterProxyModel : public BaseProxyModel
	{
		Q_OBJECT
	public:
		explicit FilterProxyModel (QObject * parent, FilterMgr * f);
		virtual ~FilterProxyModel ();

		virtual QModelIndex sibling (int row, int column, QModelIndex const & idx) const override;
		virtual bool filterAcceptsIndex (QModelIndex const & sourceIndex, QModelIndex const & sourceParent) override;
		void setSourceModel (QAbstractItemModel *sourceModel);
		virtual Qt::ItemFlags flags (QModelIndex const & index) const override;

		virtual void clearModel () override;
		virtual void clearModelData () override;

	protected:
		FilterMgr * m_filter_mgr;
	};


	class LogFilterProxyModel : public FilterProxyModel
	{
		Q_OBJECT
	public:
		explicit LogFilterProxyModel (QObject * parent, FilterMgr * f, LogTableModel * m);
		virtual ~LogFilterProxyModel ();

		//void resizeToCfg (logs::LogConfig const & config);
		void commitBatchToModel (int from, int to);
		QVariant headerData (int section, Qt::Orientation orientation, int role) const override;

	protected:
		LogTableModel * m_src_model;
		std::vector<int> m_accepted_rows_cache;
	};

	// extended filter with scopes and maybe more
	struct ExtLogFilterProxyModel : LogFilterProxyModel
	{
		bool m_scopes_enabled;
		bool m_dt_scopes_enabled;

		explicit ExtLogFilterProxyModel (QObject * parent, FilterMgr * f, LogTableModel * m);
		virtual bool filterAcceptsIndex (QModelIndex const & sourceIndex, QModelIndex const & sourceParent) override;
	};

}
