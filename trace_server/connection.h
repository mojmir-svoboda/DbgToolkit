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

#include <QHostAddress>
#include <QString>
#include <QTcpSocket>
#include <QTableView>
#include <QAbstractProxyModel>
#include <QThread>
#include <QMenu>
#include "mainwindow.h"
#include "../tlv_parser/tlv_parser.h"
#include "../tlv_parser/tlv_decoder.h"
#include "../filters/file_filter.hpp"
#include <boost/circular_buffer.hpp>
#include "sessionstate.h"
#include "filterproxy.h"
#include "cmd.h"

#include <QItemDelegate>

class Server;
class QFile;
class QDataStream;
class QStandardItemModel;
class QStandardItem;


class TableItemDelegate : public QItemDelegate
{
public: 
    TableItemDelegate (QObject *parent = 0) : QItemDelegate(parent) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    void paintCustom (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    
private slots:
    //void commitAndCloseEditor ();
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
	MainWindow const * getMainWindow () const { return m_main_window; }
	void setTableViewWidget (QTableView * w) { m_table_view_widget = w; }
	
	SessionState & sessionState () { return m_session_state; }
	SessionState const & sessionState () const { return m_session_state; }
	void loadToFileFilters (std::string const & item);
	void appendToFileFilters (std::string const & item, bool checked);
	void appendToFileFilters (boost::char_separator<char> const & sep, std::string const & item, bool checked, bool recursive);
	void appendToCtxFilters (std::string const & item, bool checked);


	void clearFilters ();
	void findText (QString const & text, tlv::tag_t tag);
	void findText (QString const & text);
	void findNext ();
	void findPrev ();
	void appendToRegexFilters (std::string const & item);
	void appendToColorRegexFilters (std::string const & item);
	void removeFromColorRegexFilters (std::string const & item);
	void recompileColorRegexps ();
	void flipFilterMode (E_FilterMode mode);
	void run ();

	typedef QList<QString> filter_color_regexs_t;

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
	void onHidePrevFromRow ();
	void onUnhidePrevFromRow ();
	void onClearCurrentView ();
	void onClearCurrentFileFilter ();
	void onClearCurrentCtxFilter ();
	void onClearCurrentTIDFilter ();
	void onClearCurrentColorizedRegexFilter ();
	void onClearCurrentScopeFilter ();
	void onShowContextMenu (QPoint const & pos);
	void onExcludeFileLine ();
	void onToggleRefFromRow ();
	void onExcludeFileLine (QModelIndex const & row_index);
	void onApplyColumnSetup ();

private slots:
	void processReadyRead ();
	void onDisconnected ();
	void onTableClicked (QModelIndex const & index);
	void onTableDoubleClicked (QModelIndex const & index);

private:
	friend class Server;
	template <class T, typename T_Ret, typename T_Arg0, typename T_Arg1>
	void processStream (T *, T_Ret (T::*read_member_t)(T_Arg0, T_Arg1));
	bool tryHandleCommand (DecodedCommand const & cmd);
	bool handleLogCommand (DecodedCommand const & cmd);
	bool handleSetupCommand (DecodedCommand const & cmd);

	bool appendToFilters (DecodedCommand const & cmd);
	void appendToFileFilters (boost::char_separator<char> const & sep, std::string const & item, bool checked);
	void appendToFileFilters (boost::char_separator<char> const & sep, std::string const & file, std::string const & line, bool checked);
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
	void setupModelCtx ();
	void setupModelTID ();
	void setupModelColorRegex ();
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
	QStandardItemModel * m_tree_view_ctx_model;
	QStandardItemModel * m_tree_view_func_model;
	QStandardItemModel * m_list_view_tid_model;
	QStandardItemModel * m_list_view_color_regex_model;
	QAbstractProxyModel * m_table_view_proxy;
	QMenu m_ctx_menu;
	QAction * m_toggle_ref;
	QAction * m_hide_prev;
	QAction * m_exclude_fileline;
	QModelIndex m_last_clicked;
	QList<QRegExp> m_color_regexps;
	std::vector<bool> m_color_regex_user_states;
	filter_color_regexs_t m_filter_color_regexs;

	// data receiving stuff
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

