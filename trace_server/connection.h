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
//#include <QAbstractProxyModel>
#include <QThread>
//#include <QMenu>
#include "mainwindow.h"
#include <tlv_parser/tlv_parser.h>
//#include <tlv_parser/tlv_decoder.h>
//#include <boost/circular_buffer.hpp>
#include "sessionstate.h"
#include "filterproxy.h"
#include "cmd.h"
#include "baseplot.h"
#include "treeview.h"

#include <QStyledItemDelegate>

class Server;
class QFile;
class QDataStream;
class QStandardItemModel;
class QStandardItem;

namespace stats { class StatsWindow; }


class TableItemDelegate : public QStyledItemDelegate
{
	SessionState const & m_session_state;
public: 
    TableItemDelegate (SessionState & ss, QObject *parent = 0) : QStyledItemDelegate(parent), m_session_state(ss) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    void paintCustom (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;

	void paintTokenized (QPainter * painter, QStyleOptionViewItemV4 & option, QModelIndex const & index, QString const & separator, QString const & out_separator, int level = 1) const;
    
private slots:
};

class LevelDelegate : public QStyledItemDelegate
{
	SessionState const & m_session_state;
public: 
    LevelDelegate (SessionState & ss, QObject *parent = 0) : QStyledItemDelegate(parent), m_session_state(ss) { }

    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    
private slots:
};



struct DataPlot {
	Connection * m_parent;
	QDockWidget * m_wd;
	plot::PlotConfig m_config;
	plot::BasePlot m_plot;
	int m_from;
	QString m_fname;

	DataPlot (Connection * parent, plot::PlotConfig & config, QString const & fname);

	void onShowPlots ()
	{
		m_wd->show();
		m_plot.onShowPlots();
	}
	void onHidePlots ()
	{
		m_wd->hide();
		m_plot.onHidePlots();
	}
};

typedef QMap<QString, DataPlot *> dataplots_t;

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
	void appendToFileTree (boost::char_separator<char> const & sep, std::string const & item, bool exclude = false);
	void appendToCtxFilters (std::string const & item, bool checked);
	void appendToLvlFilters (std::string const & item);
	void appendToLvlWidgets (FilteredLevel const & flt);

	void clearFilters ();
	void findText (QString const & text, tlv::tag_t tag);
	void findText (QString const & text);
	void findNext ();
	void findPrev ();
	void appendToRegexFilters (std::string const & str, bool checked, bool inclusive);
	void removeFromRegexFilters (std::string const & item);
	void appendToColorRegexFilters (std::string const & item);
	void removeFromColorRegexFilters (std::string const & item);
	void recompileRegexps ();
	void recompileColorRegexps ();
	void flipFilterMode (E_FilterMode mode);
	void run ();
	void loadToColorRegexps (std::string const & filter_item, std::string const & color, bool enabled);
	void loadToRegexps (std::string const & filter_item, bool inclusive, bool enabled);
	bool loadConfigForPlot (plot::PlotConfig & config, QString const & tag);
	bool saveConfigForPlot (plot::PlotConfig const & config, QString const & tag);
	bool filterEnabled () const { return m_main_window->filterEnabled(); }

signals:
	void readyForUse();
	void newMessage (QString const & from, QString const & message);
	void handleCommands ();
	
public slots:
	void onTabTraceFocus (int i);
	void onFileColOrExp (QModelIndex const &, bool collapsed);
	void onFileExpanded (QModelIndex const &);
	void onFileCollapsed (QModelIndex const &);
	void onLevelValueChanged (int i);
	QString onCopyToClipboard ();
	void setFilterFile (int state);
	void onBufferingStateChanged (int state);
	void onHandleCommands ();
	void onHandleCommandsStart ();
	void onHandleCommandsCommit ();
	void onCloseTab ();
	void onInvalidateFilter ();
	void onHidePrevFromRow ();
	void onUnhidePrevFromRow ();
	void onClearCurrentView ();
	void onClearCurrentFileFilter ();
	void onClearCurrentCtxFilter ();
	void onClearCurrentTIDFilter ();
	void onClearCurrentColorizedRegexFilter ();
	void onClearCurrentRegexFilter ();
	void onClearCurrentScopeFilter ();
	void onShowContextMenu (QPoint const & pos);
	void onShowPlotContextMenu (QPoint const &);
	void onShowPlots ();
	void onHidePlots ();
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
	friend class MainWindow;
	enum {
		e_data_ok = 0,
		e_data_pipe_full,
		e_data_decode_oor,
		e_data_decode_lnerr,
		e_data_decode_captain_failure,
		e_data_decode_general_failure,
		e_data_decode_error,
	};

	template <class T, typename T_Ret, typename T_Arg0, typename T_Arg1>
	int processStream (T *, T_Ret (T::*read_member_t)(T_Arg0, T_Arg1));
	bool tryHandleCommand (DecodedCommand const & cmd);
	bool handleLogCommand (DecodedCommand const & cmd);
	bool handleDataXYCommand (DecodedCommand const & cmd);
	bool handleDataXYZCommand (DecodedCommand const & cmd);
	bool handleSetupCommand (DecodedCommand const & cmd);
	bool handleExportCSVCommand (DecodedCommand const & cmd);
	bool handleSaveTLVCommand (DecodedCommand const & cmd);
	bool handlePingCommand (DecodedCommand const & cmd);
	bool handleShutdownCommand (DecodedCommand const & cmd);

	void appendDataXY (QString const & tag, double x, double y);
	bool appendToFilters (DecodedCommand const & cmd);
	void appendToTIDFilters (std::string const & item);
	void clearFilters (QStandardItem * node);
	void hideLinearParents ();

	GlobalConfig const & getConfig () { return m_main_window->getConfig(); }

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
	void setupModelRegex ();
	void setupModelLvl ();
	void setupColumnSizes (bool force_setup = false);
	QString findString4Tag (tlv::tag_t tag, QModelIndex const & row_index) const;
	QVariant findVariant4Tag (tlv::tag_t tag, QModelIndex const & row_index) const;
	void selectionFromTo (int & from, int & to) const;
	void findTextInAllColumns (QString const & text, int from_row, int to_row);
	void findTextInColumn (QString const & text, int col, int from_row, int to_row);

	bool isModelProxy () const;

private:
	MainWindow * m_main_window;
	SessionState m_session_state;
	int m_from_file;
	bool m_column_setup_done;
	bool m_marked_for_close;
	int m_last_search_row;
	int m_last_search_col;
	QString m_last_search;
	QTableView * m_table_view_widget;
	TreeModel * m_file_model;
	QStandardItemModel * m_ctx_model;
	QStandardItemModel * m_func_model;
	QStandardItemModel * m_tid_model;
	QStandardItemModel * m_color_regex_model;
	QStandardItemModel * m_regex_model;
	QStandardItemModel * m_lvl_model;
	QAbstractProxyModel * m_table_view_proxy;
	QMenu m_ctx_menu;
	QAction * m_toggle_ref;
	QAction * m_hide_prev;
	QAction * m_exclude_fileline;
	QModelIndex m_last_clicked;

	// data receiving stuff
	enum { e_ringbuff_size = 128 * 1024 };
	boost::circular_buffer<char> m_buffer;
	DecodedCommand m_current_cmd;
	enum { e_ringcmd_size = 16384 };
	boost::circular_buffer<DecodedCommand> m_decoded_cmds;
	tlv::TVDecoder m_decoder;
	QFile * m_storage;
	QDataStream * m_datastream;
	QTcpSocket * m_tcpstream;
	stats::StatsWindow * m_statswindow;
	dataplots_t m_dataplots;
};

inline DataPlot::DataPlot (Connection * parent, plot::PlotConfig & config, QString const & fname)
	: m_parent(parent)
	, m_wd(0)
	, m_config(config)
	, m_plot(parent, 0, m_config, fname)
	, m_from(0)
	, m_fname(fname)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

