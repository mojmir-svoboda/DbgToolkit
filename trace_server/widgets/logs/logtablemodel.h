/**
 * Copyright (C) 2011-2016 Mojmir Svoboda
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
#include <models/basetablemodel.h>
#include <widgets/findconfig.h>
#include <3rd/assocvector.h>
#include "logfile.h"

class Connection;
class FilterState;
class QAbstractProxyModel;

namespace logs {
class LogWidget; struct LogConfig;

struct LogTableModel : BaseTableModel
{
	explicit LogTableModel (QObject * parent, logs::LogWidget & lw);
	~LogTableModel ();

	virtual int rowCount (const QModelIndex & parent = QModelIndex()) const override;
	virtual int columnCount (const QModelIndex & parent = QModelIndex()) const override;
	virtual QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const override;
	virtual bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	virtual QVariant headerData (int section, Qt::Orientation orientation, int role) const override;
	virtual bool  setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role = Qt::EditRole) override;
	virtual bool checkExistence (QModelIndex const & index) const override;

	virtual void clearModel ();
	virtual void clearModelData () override;

	virtual void commitCommands (std::vector<DecodedCommand> const & dcmds, E_ReceiveMode mode) override;
	void postProcessBatch (int src_from, int src_to);
/*	virtual void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode) override;*/

	void appendCommand (DecodedCommand const & cmd);

//	void resizeToCfg (logs::LogConfig const & config);

	logs::LogWidget const & logWidget () const { return m_log_widget; }
	LogTableModel * cloneToNewModel (logs::LogWidget * parent, FindConfig const & fc);
	void reloadModelAccordingTo (logs::LogConfig & config);
	logs::proto::LogFile const & rawData () const { return m_data; }
	proto::rowdata_t getRecordDataForRow (int row) const { return m_data.getRecordDataForRow(row); }

signals:
	
public slots:

protected:
	friend class logs::LogWidget;

  logs::proto::LogFile m_data;
	logs::LogWidget & m_log_widget;
};

}
