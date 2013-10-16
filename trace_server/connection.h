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
#include "cmd.h"
#include "gantt/ganttdata.h"
#include "treeview.h"
#include "logs/logwidget.h"
#include "plot/plotwidget.h"
#include "table/tablewidget.h"
#include "gantt/ganttwidget.h"
#include "gantt/frameview.h"
#include "appdata.h"

class Server;
class QFile;
class QDataStream;
class QTextStream;
class QStandardItemModel;
class QStandardItem;
//namespace stats { class StatsWindow; }

#include <boost/mpl/apply.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/mpl/transform.hpp>

#include "constants.h"
char const * const g_fileTags[] = { g_presetLogTag, g_presetPlotTag, g_presetTableTag, g_presetGanttTag, g_presetFrameTag };

typedef boost::mpl::vector<
		boost::mpl::pair<  logs::LogWidget,    logs::LogConfig   >,
		boost::mpl::pair<  plot::PlotWidget,   plot::PlotConfig  >,
		boost::mpl::pair< table::TableWidget, table::TableConfig >,
		boost::mpl::pair< gantt::GanttWidget, gantt::GanttConfig >,
		boost::mpl::pair< FrameView,   FrameViewConfig >
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
struct DockedData : DockedWidgetBase
{
	typedef typename SelectWidget<TypeN>::type widget_t;
	typedef typename SelectConfig<TypeN>::type config_t;
	enum { e_type = TypeN };

	virtual E_DataWidgetType type () const { return static_cast<E_DataWidgetType>(e_type); }

	Connection * m_parent;
	widget_t *   m_widget;
	config_t     m_config;
	QString      m_confname;

	DockedData (Connection * parent, QString const & confname, QStringList const & dockpath)
		: DockedWidgetBase(dockpath)
		, m_parent(parent)
		, m_widget(0)
		, m_config()
		, m_confname(confname)
	{ }

	~DockedData ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		delete m_widget;
		m_widget = 0;
	}

	virtual QWidget * dockedWidget () { return m_widget; }

	widget_t & widget () { return *m_widget; }
	widget_t const & widget () const { return *m_widget; }
	config_t & config () { return m_config; }
	config_t const & config () const { return m_config; }
	virtual DockedConfigBase const & dockedConfig () const { return config(); }
	virtual DockedConfigBase & dockedConfig () { return config(); }

	virtual bool handleAction (Action * a, E_ActionHandleType sync)
	{
		switch (a->type())
		{
			case e_Visibility:
			{
				Q_ASSERT(m_args.size() > 0);
				bool const on = a->m_args.at(0).toBool();
				m_wd->setVisible(on);
				config().m_show = on;
				return true;
			}
			case e_InCentralWidget:
			{
				Q_ASSERT(m_args.size() > 0);
				bool const on = a->m_args.at(0).toBool();
				m_parent->toCentralWidget(m_wd, m_widget, on);
				config().m_central_widget = on;
				return true;
			}
			default:
				break;
		}

		return m_widget->handleAction(a, sync);
	}

	/*virtual void show ()
	{
		m_widget->onShow();
		m_wd->show();
		m_parent->getMainWindow()->restoreDockWidget(m_wd);
	}
	virtual void hide ()
	{
		m_widget->onHide();
		QTimer::singleShot(0, m_wd, SLOT(hide()));
	}*/
};

struct DataLog : DockedData<e_data_log>
{
	DataLog (Connection * parent, QString const & confname, QStringList const & path);
	~DataLog ();
};
struct DataPlot : DockedData<e_data_plot>
{
	DataPlot (Connection * parent, QString const & confname, QStringList const & path);
};
struct DataTable : DockedData<e_data_table>
{
	DataTable (Connection * parent, QString const & confname, QStringList const & path);
	~DataTable ();
};

struct DataGantt : DockedData<e_data_gantt>
{
	DataGantt (Connection * parent, QString const & confname, QStringList const & path);
};

struct DataFrame : DockedData<e_data_frame>
{
	DataFrame (Connection * parent, QString const & confname, QStringList const & path);
};


typedef boost::mpl::vector<DataLog, DataPlot, DataTable, DataGantt, DataFrame>::type dockeddata_t;
typedef boost::mpl::transform<dockeddata_t, boost::add_pointer<boost::mpl::_1> >::type dockeddataptr_t;

template <int N, typename SeqT>
struct SelectDockedData
	: boost::mpl::apply<boost::mpl::at<boost::mpl::_1, boost::mpl::_2>, SeqT, boost::mpl::int_<N> >
{ };

template <int TypeN>
struct DataMap 
	: QMap<QString, typename SelectDockedData<TypeN, dockeddataptr_t>::type >
	, ActionAble
{
	enum { e_type = TypeN };
	QList<DecodedCommand> m_queue;
	typedef typename SelectDockedData<TypeN, dockeddataptr_t>::type widget_t;

	DataMap () : ActionAble(QStringList()) { m_queue.reserve(256); }

	virtual bool handleAction (Action * a, E_ActionHandleType sync)
	{
		for (iterator it = begin(), ite = end(); it != ite; ++it)
			(*it)->handleAction(a, sync);
		return true;
	}
};

typedef DataMap<e_data_log  > datalogs_t;
typedef DataMap<e_data_plot > dataplots_t;
typedef DataMap<e_data_table> datatables_t;
typedef DataMap<e_data_gantt> datagantts_t;
typedef DataMap<e_data_frame> dataframes_t;

typedef boost::tuple<datalogs_t, dataplots_t, datatables_t, datagantts_t, dataframes_t> data_widgets_t;

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
	explicit Connection (QObject * parent = 0);
	~Connection ();

	void setMainWindow (MainWindow * w) { m_main_window = w; }
	MainWindow const * getMainWindow () const { return m_main_window; }
	MainWindow * getMainWindow () { return m_main_window; }
	
	QString getAppName () const { return m_app_name; }
	int getAppIdx () const { return m_app_idx; }
	int findAppNameInMainWindow (QString const & appname);
	AppData const & appData () const { return m_app_data; }

	void run ();

	void saveConfigs (QString const & preset_name);
	void loadConfigs (QString const & preset_name);
	void applyConfigs ();

	void requestTableSynchronization (int sync_group, unsigned long long time);
	void requestTableWheelEventSync (int sync_group, QWheelEvent * ev, QTableView const * source);
	void requestTableActionSync (int sync_group, unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QTableView const * source);

	void defaultConfigFor (logs::LogConfig & config); // loads legacy registry defaults
	template <int TypeN>
	QString getClosestPresetName (QString const & tag);

	template <int TypeN>
	typename SelectIterator<TypeN>::type
	dataWidgetFactory (QString const tag);


signals:
	void readyForUse();
	void newMessage (QString const & from, QString const & message);
	void handleCommands ();
	
public slots:
	void onTabTraceFocus ();
	void onBufferingStateChanged (int state);
	void onLevelValueChanged (int i);
	//QString onCopyToClipboard ();

	void onHandleCommands ();
	void onHandleCommandsStart ();
	void onHandleCommandsCommit ();

	//void onShowContextMenu (QPoint const & pos);
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

	void onSaveAll ();

	bool tryHandleCommand (DecodedCommand const & cmd, E_ReceiveMode mode);

	void setWidgetToTabWidget (QWidget * w);
	void unsetWidgetFromTabWidget (QWidget * & w);
	QWidget * toCentralWidget (QDockWidget * wd, QWidget * w, bool on);


private slots:
	void processReadyRead ();
	void processTailCSVStream ();
	void handleCSVSetup (QString const & fname);
	void onDisconnected ();

protected:
	friend class Server;
	friend class MainWindow;
	friend struct DataLog;
	friend class gantt::GanttWidget;
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
	bool enqueueCommand (DecodedCommand const & cmd);
	bool dequeueCommand (DecodedCommand & cmd);
	//void preparseCommand (DecodedCommand & cmd);

	bool handleSetupCommand (DecodedCommand const & cmd);
	bool handlePingCommand (DecodedCommand const & cmd);
	bool handleShutdownCommand (DecodedCommand const & cmd);
	bool handleDictionnaryCtx (DecodedCommand const & cmd);
	bool handleCSVStreamCommand (DecodedCommand const & cmd);
	bool handleExportCSVCommand (DecodedCommand const & cmd);
	bool handleSaveTLVCommand (DecodedCommand const & cmd);

	bool handleLogCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleLogClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleTableXYCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleTableSetupCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleTableClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handlePlotCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleDataXYZCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleGanttBgnCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleGanttEndCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleGanttFrameBgnCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleGanttFrameEndCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleGanttClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode);

	void registerDataMaps ();

	void convertBloodyBollockyBuggeryRegistry (logs::LogConfig & cfg);

	datalogs_t::iterator findOrCreateLog (QString const & tag);
	//void appendLog (DecodedCommand const &);

	dataplots_t::iterator findOrCreatePlot (QString const & tag);
	//void appendDataXY (double x, double y, QString const & tag);

	datatables_t::iterator findOrCreateTable (QString const & tag);
	void appendTableXY (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & tag);
	void appendTableSetup (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & hhdr, QString const & tag);

	datagantts_t::iterator findOrCreateGantt (QString const & tag);
	void appendGantt (gantt::DecodedData &);

	dataframes_t::iterator findOrCreateFrame (QString const & tag);
	void appendFrames (gantt::DecodedData &);

	GlobalConfig const & getConfig () const;

	void tryLoadMatchingPreset (QString const & appname);
	bool setupStorage (QString const & name);
	QString createStorageName () const;
	void processDataStream (QDataStream & stream);
	void copyStorageTo (QString const & filename);
	void closeStorage ();
	void setSocketDescriptor (int sd);
	void setImportFile (QString const & fname);
	void setTailFile (QString const & fname);

	void findTableIndexInFilters (QModelIndex const & row_index, bool scroll_to_item, bool expand);

	void setupModelFile ();
	void destroyModelFile ();
	void setupModelCtx ();
	void setupModelTID ();
	void setupModelColorRegex ();
	void setupModelRegex ();
	void setupModelString ();
	void setupModelLvl ();
	void setupColumnSizes (bool force_setup = false);

	bool dumpModeEnabled () const;

private:
	MainWindow * m_main_window;
	E_SrcStream m_src_stream;
	E_SrcProtocol m_src_protocol;

	AppData m_app_data;
	int m_app_idx;
	QString m_pid;
	int m_storage_idx;
	QString m_app_name;
	unsigned m_recv_bytes;
	bool m_marked_for_close;
	QString m_curr_preset;
	QWidget * m_tab_widget;
	QTextStream * m_file_csv_stream;
	QDataStream * m_file_tlv_stream;

/*
	SessionState m_session_state;
	bool m_column_setup_done;
	QString m_last_search;
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
*/


	// data receiving stuff
	enum { e_ringbuff_size = 128 * 1024 };
	boost::circular_buffer<char> m_buffer;
	DecodedCommand m_current_cmd;
	enum { e_ringcmd_size = 16384 };
	boost::circular_buffer<DecodedCommand> m_decoded_cmds;
	tlv::TVDecoder m_decoder;
	QFile * m_storage;
	QDataStream * m_tcp_dump_stream;
	QTcpSocket * m_tcpstream;
	//stats::StatsWindow * m_statswindow;
	data_widgets_t m_data;
};

#include "connection.inl"

