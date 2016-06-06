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
#include <QAbstractProxyModel>
#include <vector>
#include <3rd/assocvector.h>

class BaseProxyModel : public QAbstractProxyModel
{
	Q_OBJECT

public:
	explicit BaseProxyModel (QObject * parent);

	virtual QModelIndex index (int row, int column, QModelIndex const & parent = QModelIndex()) const override;
	virtual QModelIndex parent (QModelIndex const & child) const override { return QModelIndex(); }

	virtual int rowCount (QModelIndex const & parent = QModelIndex()) const override;
	virtual int columnCount (QModelIndex const & parent = QModelIndex()) const override;

	virtual QModelIndex mapToSource (QModelIndex const & proxyIndex) const override;

	virtual QModelIndex mapFromSource (QModelIndex const & sourceIndex) const;

	QModelIndex mapNearestFromSource (QModelIndex const & sourceIndex) const;

	bool rowInProxy (int row) const;
	int rowToSource (int row) const;

	//QVariant headerData (int section, Qt::Orientation orientation, int role) const;
	//bool  setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role = Qt::EditRole);

	virtual bool insertRows (int first, int last, QModelIndex const &) override;
	virtual bool insertColumns (int first, int last, QModelIndex const & parent = QModelIndex()) override;

	virtual QVariant data (QModelIndex const & index, int role) const override;
	virtual bool setData (QModelIndex const & index, QVariant const & value, int role) override;

	virtual Qt::ItemFlags flags (QModelIndex const & index) const override;

	virtual bool filterAcceptsIndex (QModelIndex const & sourceIndex, QModelIndex const & sourceParent) = 0;
	virtual void commitBatchToModel (int src_from, int src_to) { }

	virtual void clearModel ();
	virtual void clearModelData ();

public slots:
	void force_update();

protected:

	typedef Loki::AssocVector<int, int> map_t; // @TODO: reserve

	void insertRow (int c);

	map_t m_map_from_src;
	std::vector<int> m_map_from_tgt;
};


