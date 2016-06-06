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
#include <QAbstractTableModel>
#include <QAbstractProxyModel>
#include <trace_proto/decoder.h>
#include "types.h"
#include <QString>
#include <QColor>
#include <vector>
class BaseProxyModel;

class BaseTableModel : public QAbstractTableModel
{
	//Q_OBJECT
public:
	explicit BaseTableModel (QObject * parent, std::vector<QString> & hhdr, std::vector<int> & hsize);
	~BaseTableModel ();
	int rowCount (const QModelIndex & parent = QModelIndex()) const override = 0;
	int columnCount (const QModelIndex & parent = QModelIndex()) const override = 0;

	virtual QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const override = 0;
	virtual bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override = 0;

	virtual bool checkExistence (QModelIndex const & index) const = 0;

	virtual QVariant headerData (int section, Qt::Orientation orientation, int role) const override;
	virtual bool  setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role = Qt::EditRole) override;

	virtual void clearModel ();
	virtual void clearModelData ();

/*	virtual void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode) = 0;*/
	virtual void commitCommands (std::vector<DecodedCommand> const & dcmds, E_ReceiveMode mode) = 0;

	void emitLayoutChanged ();

	void addProxy (BaseProxyModel * pxy) { m_proxy.push_back(pxy); }
  void removeProxy (BaseProxyModel * pxy) { m_proxy.erase(std::remove(m_proxy.begin(), m_proxy.end(), pxy), m_proxy.end()); }
  void switchToProxy (BaseProxyModel * pxy);

protected:

	//virtual void commitBatchToModel (BatchCmd & batch) = 0; 
	//virtual void parseCommand (DecodedCommand const & cmd, E_ReceiveMode mode, BatchCmd & batch) = 0;
// 	unsigned long long col_time (int const col) const { return m_col_times[col]; }
	//dcmds_t const & rawData () { return m_dcmds; }

	std::vector<QString> & m_hhdr;
	std::vector<int> & m_hsize;
  std::vector<BaseProxyModel *> m_proxy;
};

