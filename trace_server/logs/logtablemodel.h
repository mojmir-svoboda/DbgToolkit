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
#include <QString>
#include <vector>
#include <tlv_parser/tlv_cmd_qstring.h>
#include <table/tablemodelview.h>
#include <filterstate.h>
#include <cmd.h>

class Connection;
class FilterState;
class QAbstractProxyModel;

namespace logs { class LogWidget; }

typedef std::vector<void const *> tree_node_ptrs_t;
typedef std::vector<unsigned> layers_t;
typedef std::vector<unsigned> row_types_t;

struct BatchCmd {
	rows_t m_rows;
	tree_node_ptrs_t m_tree_node_ptrs;

	layers_t m_layers;
	row_types_t m_rowTypes;

};

class LogTableModel : public TableModelView
{
public:
	//explicit LogTableModel (QObject * parent, QVector<QString> & hhdr, QVector<int> & hsize);
	//~TableModelView ();

	explicit LogTableModel (QObject * parent, logs::LogWidget & lw);
	~LogTableModel ();

	//void transactionStart (int n);
	//void appendCommand (DecodedCommand const & cmd);
	void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	void commitCommands (E_ReceiveMode mode);
	void appendCommand (tlv::StringCommand const & cmd);
	//void appendCommandCSV (QAbstractProxyModel * filter, tlv::StringCommand const & cmd);
	//void transactionCommit ();

	//void emitLayoutChanged ();

	//bool checkExistence (QModelIndex const & index) const;
	//bool checkColumnExistence (tlv::tag_t tag, QModelIndex const & index) const;
	//bool checkTagExistence (tlv::tag_t tag, QModelIndex const & index) const;

	layers_t const & layers () const { return m_layers; }

	row_types_t const & rowTypes () const { return m_rowTypes; }
	FilterState const & filterState () const { return m_filter_state; }

signals:
	
public slots:

protected:

	void commitBatchToModel ();
	void parseCommand (DecodedCommand const & cmd, E_ReceiveMode mode, BatchCmd & batch);

	logs::LogWidget & m_log_widget;
	FilterState & m_filter_state;
	
	BatchCmd m_batch;
	tree_node_ptrs_t m_tree_node_ptrs;
	layers_t m_layers;
	row_types_t m_rowTypes;
};

/*
	Q_OBJECT
public:
	explicit LogTableModel (QObject * parent, logs::LogWidget & lw);
	~LogTableModel ();

	int rowCount (const QModelIndex & parent = QModelIndex()) const;
	int columnCount (const QModelIndex & parent = QModelIndex()) const;
	QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const;
	bool setData (QModelIndex const & index, QVariant const & value, int role = Qt::EditRole);
	QVariant headerData (int section, Qt::Orientation orientation, int role) const;

	void transactionStart (int n);
	void appendCommand (DecodedCommand const & cmd);
	void appendCommand (QAbstractProxyModel * filter, tlv::StringCommand const & cmd);
	void appendCommandCSV (QAbstractProxyModel * filter, tlv::StringCommand const & cmd);
	void transactionCommit ();

	void emitLayoutChanged ();

	bool checkExistence (QModelIndex const & index) const;
	bool checkColumnExistence (tlv::tag_t tag, QModelIndex const & index) const;
	bool checkTagExistence (tlv::tag_t tag, QModelIndex const & index) const;

	typedef std::vector<unsigned> layers_t;
	layers_t const & layers () const { return m_layers; }

	typedef std::vector<unsigned> row_types_t;
	row_types_t const & rowTypes () const { return m_rowTypes; }

	FilterState const & filterState () const { return m_filter_state; }

signals:
	
public slots:

private:

	logs::LogWidget & m_log_widget;
	FilterState & m_filter_state;
	typedef std::vector<QString> columns_t;
	typedef std::vector<columns_t> rows_t;
	rows_t m_rows;
	typedef std::vector<void const *> tree_node_ptrs_t;
	tree_node_ptrs_t m_tree_node_ptrs;

	layers_t m_layers;
	row_types_t m_rowTypes;
};
*/

