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
#include <tlv_parser/tlv_parser.h>

class TableModelView : public QAbstractTableModel
{
	//Q_OBJECT
public:
	explicit TableModelView (QObject * parent = 0);
	~TableModelView ();
	int rowCount (const QModelIndex & parent = QModelIndex()) const;
	int columnCount (const QModelIndex & parent = QModelIndex()) const;
	QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const;
	bool setData (QModelIndex const & index, QVariant const & value, int role = Qt::EditRole);
	QVariant headerData (int section, Qt::Orientation orientation, int role) const;

	void transactionStart (size_t n);
	//void appendCommand (tlv::StringCommand const & cmd);
	void appendTableXY (int x, int y, QString const & msg_tag);
	void transactionCommit ();

	void emitLayoutChanged ();

	bool checkExistence (QModelIndex const & index) const;

private:

	typedef std::vector<QString> columns_t;
	typedef std::vector<columns_t> rows_t;
	rows_t m_rows;
};

