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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QComboBox>
#include <QSystemTrayIcon>
#include "server.h"
#include "settings.h"
#include <tlv_parser/tlv_parser.h>
#include "config.h"
#include "dock.h"

namespace Ui {
	class MainWindow;
	class SettingsDialog;
	class HelpDialog;
}

class QSystemTrayIcon;
class QAction;
class QMenu;
class QListView;
class QStandardItemModel;
class QLabel;
class SessionState;
class TreeView;

enum E_ApiErrors { e_InvalidItem = -1 };

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow (QWidget * parent, bool quit_delay, bool dump_mode, QString const & name);
	~MainWindow ();

	QTabWidget * getTabTrace ();
	QTabWidget const * getTabTrace () const;

	QString getAppDir () const { return m_config.m_appdir; }

	columns_setup_t const & getColumnSetup (size_t i) const { return m_config.m_columns_setup.at(i); }
	columns_setup_t & getColumnSetup (size_t i) { return m_config.m_columns_setup[i]; }
	columns_sizes_t const & getColumnSizes (size_t i) const { return m_config.m_columns_sizes.at(i); }
	columns_sizes_t & getColumnSizes (size_t i) { return m_config.m_columns_sizes[i]; }
	columns_align_t const & getColumnAlign (size_t i) const { return m_config.m_columns_align.at(i); }
	columns_align_t & getColumnAlign (size_t i) { return m_config.m_columns_align[i]; }
	columns_elide_t const & getColumnElide (size_t i) const { return m_config.m_columns_elide.at(i); }
	columns_align_t & getColumnElide (size_t i) { return m_config.m_columns_elide[i]; }

	int findPresetName (QString const & name)
	{
		for (size_t i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
			if (m_config.m_preset_names[i] == name)
				return i;
		return e_InvalidItem;
	}
	

	void saveLayout (QString const & preset_name);
	void loadLayout (QString const & preset_name);
	void saveSession (SessionState const & s, QString const & preset_name) const;
	bool loadSession (SessionState & s, QString const & preset_name);
	size_t addPresetName (QString const & name)
	{
		m_config.m_preset_names.push_back(name);
		return m_config.m_preset_names.size() - 1;
	}

	int createAppName (QString const & appname, E_SrcProtocol const proto);
	void addNewApplication (QString const & appname);
	int findAppName (QString const & appname)
	{
		for (int i = 0, ie = m_config.m_app_names.size(); i < ie; ++i)
			if (m_config.m_app_names[i] == appname)
				return i;
		return e_InvalidItem;
	}
	QList<QColor> const & getThreadColors () const { return m_config.m_thread_colors; }
	TreeView * getWidgetFile ();
	TreeView const * getWidgetFile () const;
	QTreeView * getWidgetCtx ();
	QTreeView const * getWidgetCtx () const;
    QComboBox * getFilterRegex ();
    QComboBox const * getFilterRegex () const;
	QTreeView * getWidgetRegex ();
	QTreeView const * getWidgetRegex () const;
	QTreeView * getWidgetString ();
	QTreeView const * getWidgetString () const;
    QComboBox * getFilterColorRegex ();
    QComboBox const * getFilterColorRegex () const;
	QListView * getWidgetColorRegex ();
	QListView const * getWidgetColorRegex () const;
	QListView * getWidgetTID ();
	QListView const * getWidgetTID () const;
	QTreeView * getWidgetLvl ();
	QTreeView const * getWidgetLvl () const;
	QTreeView const * getWidgetPlots () const;
	void setLevel (int i);
	int getLevel () const;
	bool dumpModeEnabled () const { return m_config.m_dump_mode; }
	bool scopesEnabled () const;
	bool indentEnabled () const;
	bool cutPathEnabled () const;
	bool cutNamespaceEnabled () const;
	int cutPathLevel () const;
	int cutNamespaceLevel () const;
	int indentLevel () const;
	int tableRowSize () const;
	QString tableFont () const;
	bool onTopEnabled () const;
	bool autoScrollEnabled () const;
	bool reuseTabEnabled () const;
	bool filterEnabled () const;
	bool plotEnabled () const;
	bool tableEnabled () const;
	bool buffEnabled () const;
	Qt::CheckState buffState () const;
	bool clrFltEnabled () const;
	bool statsEnabled () const;
	bool filterPaneVertical () const;
	bool dtEnabled () const;
	void changeEvent (QEvent* e);
	void dropEvent (QDropEvent * event);
	void dragEnterEvent (QDragEnterEvent *event);
	bool eventFilter (QObject * o, QEvent * e);
	void setPresetNameIntoComboBox (QString const & pname);
	void onPresetActivate (QString const & pname);
	void onPresetActivate (Connection * conn, QString const & pname);

	QString getCurrentPresetName () const;
	QString promptAndCreatePresetName ();
	QString getValidCurrentPresetName ();

	unsigned getHotKey () const;
	Server const * getServer () const { return m_server; }
	Server * getServer () { return m_server; }
	GlobalConfig const & getConfig () const { return m_config; }

public slots:
	void onHotkeyShowOrHide ();
	void hide ();
	void showNormal ();
	void showMaximized ();
	void onOnTop (int);

	friend class Connection;
private slots:
	void loadState ();
	void loadPresets ();
	void storeGeometry ();
	void storeState ();
	void saveCurrentSession (QString const & preset_name);
	void storePresets ();
	void storePresetNames ();
	void timerHit ();
	void onQuit ();
	void onEditFind ();
	void onEditFindNext ();
	void onEditFindPrev ();
	void onFileLoad ();
	void openFiles (QStringList const & list);
	void onFileTail ();
	void onLogTail ();
	void tailFiles (QStringList const & list);
	void onFileSave ();
	void onFileExportToCSV ();
	void closeEvent (QCloseEvent *event);
	void iconActivated (QSystemTrayIcon::ActivationReason reason);
	void onQSearch (QString const &);
	void onQSearchEditingFinished ();
	void onQFilterLineEditFinished ();
	void appendToSearchHistory (QString const & str);
	void updateSearchHistory ();
	void onSaveCurrentState ();
	void onSaveCurrentStateTo (QString const & name);
	void onAddCurrentState ();
	void onRmCurrentState ();
	void onPresetActivate (int idx);
	void onPresetActivate ();
	void onRegexActivate (int idx);
	void onRegexAdd ();
	void onRegexRm ();
	void onColorRegexActivate (int idx);
	void onColorRegexAdd ();
	void onColorRegexRm ();
	//void onStringActivate (int idx);
	void onStringAdd ();
	void onStringRm ();
	void onShowHelp ();
	void onDumpFilters ();
	void onReuseTabChanged (int state);
	void ondtToolButton ();
	void onTableFontToolButton ();
	void onPlotStateChanged (int state);
	void onSaveAllButton ();
	void onPlotsToolButton ();
	void onPlotsClosed ();
	void onTablesStateChanged (int state);
	void onDockRestoreButton ();
	void onFilterFile (int state);

	void onSetupAction ();
	void prepareSettingsWidgets (int idx, bool first_time = false);
	void clearSettingWidgets ();
	void setupSeparatorChar (QString const & c);
	QString separatorChar () const;
	void onSetup (E_SrcProtocol const proto, int curr_app_idx = -1, bool first_time = false, bool mac_user = false);
	//void onSetupCSV (int curr_app_idx = -1, bool first_time = false, bool mac_user = false);
	//void onSetupCSVSeparator (int curr_app_idx = -1, int column = -1, bool first_time = false);
	void onSetupCSVColumns (int curr_app_idx, int columns, bool first_time);
	void onSetupCSVSeparator (int curr_app_idx, bool first_time);
	void onSettingsAppSelectedTLV (int idx, bool first_time = false);
	void onSettingsAppSelectedCSV (int idx, int columns, bool first_time = false);
	void settingsDisableButSeparator ();
	void settingsToDefault ();
	void onClickedAtSettingPooftahButton ();
	void onClickedAtSettingOkButton ();
	void onClickedAtSettingOkSaveButton ();
	void onClickedAtSettingCancelButton ();
	void onClickedAtSettingColumnSetup (QModelIndex idx);
	void onClickedAtSettingColumnSizes (QModelIndex idx);
	void onClickedAtSettingColumnAlign (QModelIndex idx);
	void onClickedAtSettingColumnElide (QModelIndex idx);
	void onGotoFileFilter ();
	void onGotoColorFilter ();
	void onGotoRegexFilter ();
	void onGotoLevelFilter ();
	void syncSettingsViews (QListView const * const invoker, QModelIndex const idx);

private:
	void showServerStatus ();
	void loadNetworkSettings ();
	void setupMenuBar ();
	void createActions ();
	void createTrayIcon ();

	Ui::MainWindow * ui;
	Ui::SettingsDialog * ui_settings;
	Ui::HelpDialog * m_help;
	GlobalConfig m_config;

	QTimer * m_timer;
	Server * m_server;
	QAction * m_minimize_action;
	QAction * m_maximize_action;
	QAction * m_restore_action;
	QAction * m_quit_action;
	QMenu * m_tray_menu;
	QSystemTrayIcon * m_tray_icon;
	QLabel * m_status_label;
	QDialog * m_settings_dialog;
	DockManager m_dock_mgr;
	DockWidget * m_plots_dock;
	TreeView * m_plot_tree_view;
	QString m_log_name;
};

#endif // MAINWINDOW_H
