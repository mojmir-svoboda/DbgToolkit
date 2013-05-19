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
#include <QThread>
#include <QAbstractItemView>
#include "mainwindow.h"
#include <tlv_parser/tlv.h>
#include <tlv_parser/tlv_cmd_qstring.h>
#include <boost/circular_buffer.hpp>
#include <boost/tuple/tuple.hpp>
#include "sessionstate.h"
#include "logs/filterproxy.h"
#include "cmd.h"
#include "plot/plotwidget.h"
#include "table/tablewidget.h"
#include "gantt/ganttwidget.h"
#include "gantt/ganttdata.h"
#include "treeview.h"
#include "treeproxy.h"
#include "logs/logwidget.h"

class Server;
class QFile;
class QDataStream;
class QTextStream;
class QStandardItemModel;
class QStandardItem;
class LevelDelegate;
class CtxDelegate;
class StringDelegate;
class RegexDelegate;
//namespace stats { class StatsWindow; }

#include <boost/mpl/apply.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/pair.hpp>

enum E_DataWidgetType {
	  e_data_log
	, e_data_plot
	, e_data_table
	, e_data_gantt
	, e_data_widget_max_value
};

#include "constants.h"
char const * const g_fileTags[] = { g_presetLogTag, g_presetPlotTag, g_presetTableTag, g_presetGanttTag };

typedef boost::mpl::vector<
		boost::mpl::pair<  logs::LogWidget,    logs::LogConfig   >,
		boost::mpl::pair<  plot::PlotWidget,   plot::PlotConfig  >,
		boost::mpl::pair< table::TableWidget, table::TableConfig >,
		boost::mpl::pair< gantt::GanttWidget, gantt::GanttConfig >
	>::type datawidgetcfgs_t;

template <int N, typename SeqT>
struct SelectInternals
	: boost::mpl::apply<boost::mpl::at<boost::mpl::_1, boost::mpl::_2>, SeqT, boost::mpl::int_<N> >
{ };

template <int N>
struct SelectWidget
	: boost::mpl::apply<boost::mpl::first<boost::mpl::_1>, typename SelectInternals<N, datawidgetcfgs_t>::type >
{ };
template <int N>
struct SelectConfig
	: boost::mpl::apply<boost::mpl::second<boost::mpl::_1>, typename SelectInternals<N, datawidgetcfgs_t>::type >
{ };


template <int TypeN>
struct DockedData
{
	typedef typename SelectWidget<TypeN>::type widget_t;
	typedef typename SelectConfig<TypeN>::type config_t;
	enum { e_type = TypeN };

	Connection * m_parent;
	QDockWidget * m_wd;
	widget_t * m_widget;
	config_t m_config;
	QString m_fname;

	DockedData (Connection * parent, config_t & config, QString const & fname)
		: m_parent(parent) , m_wd(0) , m_widget(0) , m_config(config) , m_fname(fname)
	{ }

	~DockedData ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		delete m_widget;
		m_widget = 0;
	}

	widget_t & widget () { return *m_widget; }
	widget_t const & widget () const { return *m_widget; }
	config_t & config () { return m_config; }
	config_t const & config () const { return m_config; }


	void onShow ()
	{
		m_widget->onShow();
		m_wd->show();
		m_parent->getMainWindow()->restoreDockWidget(m_wd);
	}
	void onHide ()
	{
		m_widget->onHide();
		QTimer::singleShot(0, m_wd, SLOT(hide()));
	}
};

struct DataLog : DockedData<e_data_log>
{
	DataLog (Connection * parent, config_t & config, QString const & fname);
};
struct DataPlot : DockedData<e_data_plot>
{
	DataPlot (Connection * parent, config_t & config, QString const & fname);
};
struct DataTable : DockedData<e_data_table>
{
	DataTable (Connection * parent, config_t & config, QString const & fname);
};

struct DataGantt : DockedData<e_data_gantt>
{
	DataGantt (Connection * parent, config_t & config, QString const & fname);
};

typedef boost::mpl::vector<DataLog *, DataPlot *, DataTable *, DataGantt *>::type dockeddata_t;

template <int N>
struct SelectDockedData
	: boost::mpl::apply<boost::mpl::at<boost::mpl::_1, boost::mpl::_2>, dockeddata_t, boost::mpl::int_<N> >
{ };

template <int TypeN>
struct DataMap : public QMap<QString, typename SelectDockedData<TypeN>::type >
{
	enum { e_type = TypeN };
	QList<DecodedCommand> m_queue;

	DataMap () { m_queue.reserve(256); }
};

typedef DataMap<e_data_log  > datalogs_t;
typedef DataMap<e_data_plot > dataplots_t;
typedef DataMap<e_data_table> datatables_t;
typedef DataMap<e_data_gantt> datagantts_t;

typedef boost::tuple<datalogs_t, dataplots_t, datatables_t, datagantts_t> data_widgets_t;

template <int TypeN>
struct SelectIterator
{
	typedef typename SelectWidget<TypeN>::type widget_t;
	typedef typename DataMap<TypeN>::iterator type;
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
	MainWindow * getMainWindow () { return m_main_window; }
	void setTableViewWidget (QTableView * w) { m_table_view_widget = w; }
	QTableView const * getTableViewWidget () const { return m_table_view_widget; }
	QTableView * getTableViewWidget () { return m_table_view_widget; }
	
	SessionState & sessionState () { return m_session_state; }
	SessionState const & sessionState () const { return m_session_state; }
	void appendToCtxFilters (QString const & item, bool checked);
	void appendToCtxWidgets (FilteredContext const & flt);
	void appendToLvlFilters (QString const & item);
	void appendToLvlWidgets (FilteredLevel const & flt);
	void appendToStringWidgets (FilteredString const & flt);

	void clearFilters ();
	void findText (QString const & text, tlv::tag_t tag);
	void findText (QString const & text);
	void findAllTexts (QString const & text);
	void findNext (QString const & text);
	void findPrev (QString const & text);
	void scrollToCurrentTag ();
	void scrollToCurrentSelection ();
	void appendToStringFilters (QString const & str, bool checked, int state);
	void removeFromStringFilters (QString const & item);
	void recompileStrings ();
	void appendToRegexFilters (QString const & str, bool checked, bool inclusive);
	void removeFromRegexFilters (QString const & item);
	void appendToColorRegexFilters (QString const & item);
	void removeFromColorRegexFilters (QString const & item);
	void recompileRegexps ();
	void recompileColorRegexps ();
	void flipFilterMode (E_FilterMode mode);
	void run ();
	void loadToColorRegexps (QString const & filter_item, QString const & color, bool enabled);
	void loadToRegexps (QString const & filter_item, bool inclusive, bool enabled);
	bool loadConfigForPlot (QString const & preset_name, plot::PlotConfig & config, QString const & tag);
	bool saveConfigForPlot (plot::PlotConfig const & config, QString const & tag);
	bool loadConfigForPlots (QString const & preset_name);
	bool saveConfigForPlots (QString const & preset_name);
	bool loadConfigForTable (QString const & preset_name, table::TableConfig & config, QString const & tag);
	bool saveConfigForTable (table::TableConfig const & config, QString const & tag);
	bool loadConfigForTables (QString const & preset_name);
	bool saveConfigForTables (QString const & preset_name);
	bool loadConfigForGantt (QString const & preset_name, gantt::GanttConfig & config, QString const & tag);
	bool saveConfigForGantt (gantt::GanttConfig const & config, QString const & tag);
	bool loadConfigForGantts (QString const & preset_name);
	bool saveConfigForGantts (QString const & preset_name);
	//bool loadConfigForLog (QString const & preset_name, logs::LogConfig & config, QString const & tag);
	bool saveConfigForLog (logs::LogConfig const & config, QString const & tag);
	bool loadConfigForLogs (QString const & preset_name);
	bool saveConfigForLogs (QString const & preset_name);

	bool filterEnabled () const { return m_main_window->filterEnabled(); }

	void requestTableSynchronization (int sync_group, unsigned long long time);
	void requestTableWheelEventSync (int sync_group, QWheelEvent * ev, QTableView const * source);
	void requestTableActionSync (int sync_group, unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QTableView const * source);

	QAbstractTableModel const * modelView () const { return static_cast<QAbstractTableModel const *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model()); }
	QAbstractProxyModel const * proxyView () const { return static_cast<QAbstractProxyModel const *>(m_table_view_proxy); }

	bool isModelProxy () const;
	TreeModel * fileModel () { return m_file_model; }
	TreeModel const * fileModel () const { return m_file_model; }
	void onSectionResized (int logicalIndex, int oldSize, int newSize);

signals:
	void readyForUse();
	void newMessage (QString const & from, QString const & message);
	void handleCommands ();
	
public slots:
	void onTabTraceFocus ();
	void onFileColOrExp (QModelIndex const &, bool collapsed);
	void onFileExpanded (QModelIndex const &);
	void onFileCollapsed (QModelIndex const &);
	void onLevelValueChanged (int i);
	QString onCopyToClipboard ();
	void nextToView ();
	void scrollToCurrentTagOrSelection ();
	void syncSelection (QModelIndexList const & from);
	void setFilterFile (int state);
	void onBufferingStateChanged (int state);
	void onHandleCommands ();
	void onHandleCommandsStart ();
	void onHandleCommandsCommit ();
	void onInvalidateFilter ();
	void onHidePrevFromRow ();
	void onUnhidePrevFromRow ();
	void onClearCurrentView ();
	void onClearCurrentFileFilter ();
	void onClearCurrentCtxFilter ();
	void onClearCurrentTIDFilter ();
	void onClearCurrentColorizedRegexFilter ();
	void onClearCurrentRegexFilter ();
	void onClearCurrentStringFilter ();
	void onClearCurrentScopeFilter ();
	void onClearCurrentRefTime ();
	void onShowContextMenu (QPoint const & pos);
	void onShowPlotContextMenu (QPoint const &);
	void onShowPlots ();
	void onHidePlots ();
	void onShowTableContextMenu (QPoint const &);
	void onShowTables ();
	void onHideTables ();
	void onShowGanttContextMenu (QPoint const &);
	void onShowGantts ();
	void onHideGantts ();
	void onShowLogContextMenu (QPoint const &);
	void onShowLogs ();
	void onHideLogs ();
	void onExcludeFileLine ();
	void onToggleRefFromRow ();
	void onColorTagRow (int row);
	void onExcludeFileLine (QModelIndex const & row_index);
	void onFindFileLine (QModelIndex const & row_index);
	void onApplyColumnSetup ();
	void onColorRegexChanged();
	void onSaveAll ();
	void onFilterFileComboChanged (QString str);
	void onCancelFilterFileButton ();
	void onCutParentValueChanged (int i);
	void onCollapseChilds ();

private slots:
	void processReadyRead ();
	void processTailCSVStream ();
	void handleCSVSetup (QString const & fname);
	void onDisconnected ();
	void onTableClicked (QModelIndex const & index);
	void onTableDoubleClicked (QModelIndex const & index);
	void findTableIndexInFilters (QModelIndex const & row_index, bool scroll_to_item, bool expand);

private:
	friend class Server;
	friend class MainWindow;
	friend class DataLog;
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
	bool queueCommand (DecodedCommand const & cmd);
	bool tryHandleCommand (DecodedCommand const & cmd);
	bool handleLogCommand (DecodedCommand const & cmd);
	bool handleTableXYCommand (DecodedCommand const & cmd);
	bool handleTableSetupCommand (DecodedCommand const & cmd);
	bool handleDataXYCommand (DecodedCommand const & cmd);
	bool handleDataXYZCommand (DecodedCommand const & cmd);
	bool handleSetupCommand (DecodedCommand const & cmd);
	bool handleExportCSVCommand (DecodedCommand const & cmd);
	bool handleSaveTLVCommand (DecodedCommand const & cmd);
	bool handlePingCommand (DecodedCommand const & cmd);
	bool handleShutdownCommand (DecodedCommand const & cmd);
	bool handleDictionnaryCtx (DecodedCommand const & cmd);
	bool handleCSVStreamCommand (DecodedCommand const & cmd);
	bool handleGanttBgnCommand (DecodedCommand const & cmd);
	bool handleGanttEndCommand (DecodedCommand const & cmd);
	bool handleGanttFrameBgnCommand (DecodedCommand const & cmd);
	bool handleGanttFrameEndCommand (DecodedCommand const & cmd);
	bool handleGanttClearCommand (DecodedCommand const & cmd);
	bool handlePlotClearCommand (DecodedCommand const & cmd);
	bool handleTableClearCommand (DecodedCommand const & cmd);
	bool handleLogClearCommand (DecodedCommand const & cmd);

	template <int TypeN>
	typename SelectIterator<TypeN>::type
	dataWidgetFactory (QString const tag);

	template <int TypeN>
	bool loadConfigFor (QString const & preset_name, typename SelectConfig<TypeN>::type & config, QString const & tag);

	void appendDataXY (double x, double y, QString const & tag);
	datatables_t::iterator findOrCreateTable (QString const & tag);
	void appendTableXY (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & tag);
	void appendTableSetup (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & hhdr, QString const & tag);

	datagantts_t::iterator findOrCreateGantt (QString const & tag);
	void appendGantt (gantt::DecodedData &);

	datalogs_t::iterator findOrCreateLog (QString const & tag);
	dataplots_t::iterator findOrCreatePlot (QString const & tag);
	//void appendLog (logs::DecodedData &);

	bool appendToFilters (DecodedCommand const & cmd);
	void appendToTIDFilters (QString const & item);
	void clearFilters (QStandardItem * node);

	GlobalConfig const & getConfig () { return m_main_window->getConfig(); }

	void tryLoadMatchingPreset (QString const & appname);
	bool setupStorage (QString const & name);
	QString createStorageName () const;
	void processDataStream (QDataStream & stream);
	void copyStorageTo (QString const & filename);
	void exportStorageToCSV (QString const & filename);
	void closeStorage ();
	void setSocketDescriptor (int sd);
	void setImportFile (QString const & fname);
	void setTailFile (QString const & fname);
	void setupModelFile ();
	void destroyModelFile ();
	void setupModelCtx ();
	void setupModelTID ();
	void setupModelColorRegex ();
	void setupModelRegex ();
	void setupModelString ();
	void setupModelLvl ();
	void setupColumnSizes (bool force_setup = false);
	QString findString4Tag (tlv::tag_t tag, QModelIndex const & row_index) const;
	QVariant findVariant4Tag (tlv::tag_t tag, QModelIndex const & row_index) const;
	void selectionFromTo (int & from, int & to) const;
	void findTextInAllColumns (QString const & text, int from_row, int to_row, bool only_first);
	void findTextInColumn (QString const & text, int col, int from_row, int to_row);
	void findTextInColumnRev (QString const & text, int col, int from_row, int to_row);
	bool matchTextInCell (QString const & text, int row, int col);
	void endOfSearch ();

private:
	MainWindow * m_main_window;
	SessionState m_session_state;
	E_SrcStream m_src_stream;
	E_SrcProtocol m_src_protocol;
	bool m_column_setup_done;
	bool m_marked_for_close;
	int m_last_search_row;
	int m_last_search_col;
	QString m_last_search;
	QString m_curr_preset;

	QWidget * m_tab_widget;
	QTableView * m_table_view_widget;
	TreeModel * m_file_model;
	TreeProxyModel * m_file_proxy;
	QItemSelectionModel * m_proxy_selection;
	QStandardItemModel * m_ctx_model;
	QStandardItemModel * m_func_model;
	QStandardItemModel * m_tid_model;
	QStandardItemModel * m_color_regex_model;
	QStandardItemModel * m_regex_model;
	QStandardItemModel * m_lvl_model;
	QStandardItemModel * m_string_model;

	enum E_Delegates {
		  e_delegate_Level
		, e_delegate_Ctx
		, e_delegate_String
		, e_delegate_Regex
		, e_delegate_max_enum_value
	};
	boost::tuple<LevelDelegate *, CtxDelegate *, StringDelegate *, RegexDelegate *> m_delegates;
	QAbstractProxyModel * m_table_view_proxy;
	QAbstractItemModel * m_table_view_src;

	QMenu m_ctx_menu;
	enum E_Actions {
		  e_action_ToggleRef
		, e_action_HidePrev
		, e_action_ExcludeFileLine
		, e_action_Find
		, e_action_Copy
		, e_action_ColorTag
		, e_action_Setup
		, e_action_max_enum_value
	};
	std::vector<QAction *> m_actions;
	QModelIndex m_last_clicked;

	// data receiving stuff
	enum { e_ringbuff_size = 128 * 1024 };
	boost::circular_buffer<char> m_buffer;
	DecodedCommand m_current_cmd;
	enum { e_ringcmd_size = 16384 };
	boost::circular_buffer<DecodedCommand> m_decoded_cmds;
	tlv::TVDecoder m_decoder;
	QFile * m_storage;
	QDataStream * m_tcp_dump_stream;
	QTextStream * m_file_csv_stream;
	QTcpSocket * m_tcpstream;
	//stats::StatsWindow * m_statswindow;
	data_widgets_t m_data;
	TreeModel * m_data_model;
};

#include "connection.inl"

