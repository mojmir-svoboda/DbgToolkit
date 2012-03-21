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
#ifndef CONNECTION_H
#define CONNECTION_H

#include <QHostAddress>
#include <QString>
#include <QTcpSocket>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QThread>
#include "mainwindow.h"
#include "../tlv_parser/tlv_parser.h"
#include "../tlv_parser/tlv_decoder.h"
#include "../filters/file_filter.hpp"
#include <boost/circular_buffer.hpp>
#include "sessionstate.h"

class Server;
class QFile;
class QDataStream;
class QStandardItemModel;
class QStandardItem;

struct DecodedCommand : tlv::StringCommand
{
	enum { e_max_sz = 2048 };
	char orig_message[e_max_sz];
	bool written_hdr;
	bool written_payload;

	DecodedCommand () : StringCommand() { Reset(); }

	void Reset ()
	{
		written_hdr = written_payload = false;
		hdr.Reset();
		tvs.clear();
	}
};

class FilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	FilterProxyModel (QObject * parent, QList<QRegExp> const & r, SessionState & ss);

public slots:
	void force_update();

protected:
	bool filterAcceptsRow (int sourceRow, QModelIndex const & sourceParent) const;
	SessionState & m_session_state;
	QList<QRegExp> const & m_regexps;
};


/**@class		Connection
 * @brief		represents incoming connection (or file stream)
 */
class Connection : public QThread
{
	Q_OBJECT
public:
	explicit Connection(QObject *parent = 0);
	~Connection ();
	void setMainWindow (MainWindow * w) { m_main_window = w; }
	void setTableViewWidget (QTableView * w) { m_table_view_widget = w; }
	
	SessionState & sessionState () { return m_session_state; }
	SessionState const & sessionState () const { return m_session_state; }
	void appendToFileFilters (std::string const & item, bool checked = false);
	void clearFilters ();
	void findText (QString const & text, tlv::tag_t tag);
	void findText (QString const & text);
	void findNext ();
	void findPrev ();
	void appendToRegexFilters (std::string const & item);

	void run ();

signals:
	void readyForUse();
	void newMessage (QString const & from, QString const & message);
	void handleCommands ();
	
public slots:
	void onTabTraceFocus (int i);
	void onLevelValueChanged (int i);
	QString onCopyToClipboard ();
	void setFilterFile (int state);
	void onBufferingStateChanged (int state);
	void onHandleCommands ();
	void onCloseTab ();
	void onInvalidateFilter ();

private slots:
	void processReadyRead ();
	void onDisconnected ();
	void onTableClicked (QModelIndex const & index);

private:
	friend class Server;
	template <class T, typename T_Ret, typename T_Arg0, typename T_Arg1>
	void processStream (T *, T_Ret (T::*read_member_t)(T_Arg0, T_Arg1));
	bool tryHandleCommand (DecodedCommand const & cmd);
	bool handleLogCommand (DecodedCommand const & cmd);
	bool handleSetupCommand (DecodedCommand const & cmd);

	bool appendToFilters (DecodedCommand const & cmd);
	void appendToFileFilters (boost::char_separator<char> const & sep, std::string const & item, bool checked = false);
	void appendToFileFilters (boost::char_separator<char> const & sep, std::string const & file, std::string const & line);
	void appendToTIDFilters (std::string const & item);
	void clearFilters (QStandardItem * node);
	void hideLinearParents ();

	bool setupStorage (QString const & name);
	QString createStorageName () const;
	void processDataStream (QDataStream & stream);
	void copyStorageTo (QString const & filename);
	void exportStorageToCSV (QString const & filename);
	void closeStorage ();
	void setSocketDescriptor (int sd);
	void setupModelFile ();
	void setupModelTID ();
	QString findString4Tag (tlv::tag_t tag, QModelIndex const & row_index) const;
	QVariant findVariant4Tag (tlv::tag_t tag, QModelIndex const & row_index) const;
	void selectionFromTo (int & from, int & to) const;
	void findTextInAllColumns (QString const & text, int from_row, int to_row);
	void findTextInColumn (QString const & text, int col, int from_row, int to_row);

private:
	MainWindow * m_main_window;
	SessionState m_session_state;
	int m_from_file;
	bool m_first_line;
	int m_last_search_row;
	int m_last_search_col;
	QString m_last_search;
	QTableView * m_table_view_widget;
	QStandardItemModel * m_tree_view_file_model;
	QStandardItemModel * m_tree_view_func_model;
	QStandardItemModel * m_list_view_tid_model;
	QSortFilterProxyModel * m_table_view_proxy;

	enum { e_ringbuff_size = 16 * 1024 };
	boost::circular_buffer<char> m_buffer;
	DecodedCommand m_current_cmd;
	enum { e_ringcmd_size = 128 };
	boost::circular_buffer<DecodedCommand> m_decoded_cmds;
	tlv::TVDecoder m_decoder;
	QFile * m_storage;
	QDataStream * m_datastream;
	QTcpSocket * m_tcpstream;
};

#endif // CONNECTION_H
