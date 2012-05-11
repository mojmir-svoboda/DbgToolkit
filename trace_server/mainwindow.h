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
#include <QTreeView>
#include <QSystemTrayIcon>
#include "server.h"
#include "types.h"

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

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget * parent = 0, bool quit_delay = true);
	~MainWindow();

	QTabWidget * getTabTrace ();
	QTabWidget const * getTabTrace () const;

	columns_setup_t const & getColumnSetup (size_t i) const { return m_columns_setup.at(i); }
	columns_setup_t & getColumnSetup (size_t i) { return m_columns_setup[i]; }
	columns_sizes_t const & getColumnSizes (size_t i) const { return m_columns_sizes.at(i); }
	columns_sizes_t & getColumnSizes (size_t i) { return m_columns_sizes[i]; }
	QList<QRegExp> const & getRegexps () const { return m_regexps; }
	std::vector<bool> const & getRegexUserStates () const { return m_regex_user_states; }
	filter_regexs_t const & getFilterRegexs () const { return m_filter_regexs; }
	filter_regexs_t & getFilterRegexs () { return m_filter_regexs; }

	filter_preset_t const & getFilterPresets (size_t i) const { return m_filter_presets.at(i); }
	filter_preset_t & getFilterPresets (size_t i) { return m_filter_presets[i]; }
	int findPresetName (QString const & name)
	{
		for (size_t i = 0, ie = m_preset_names.size(); i < ie; ++i)
			if (m_preset_names[i] == name)
				return i;
		return -1;
	}
	
	size_t addPresetName (QString const & name)
	{
		m_preset_names.push_back(name);
		m_filter_presets.push_back(filter_preset_t());
		m_filter_presets.back().reserve(32);
		return m_preset_names.size() - 1;
	}

	size_t findAppName (QString const & appname)
	{
		for (size_t i = 0, ie = m_app_names.size(); i < ie; ++i)
			if (m_app_names[i] == appname)
				return i;
		
		m_app_names.push_back(appname);
		m_columns_setup.reserve(16);
		m_columns_setup.push_back(columns_setup_t());
		m_columns_sizes.reserve(16);
		m_columns_sizes.push_back(columns_sizes_t());
		return m_app_names.size() - 1;
	}
	QList<QColor> const & getThreadColors () const { return m_thread_colors; }
	QTreeView * getTreeViewFile ();
	QTreeView const * getTreeViewFile () const;
	QTreeView * getTreeViewCtx ();
	QTreeView const * getTreeViewCtx () const;
    QComboBox * getFilterRegex ();
    QComboBox const * getFilterRegex () const;
	QListView * getListViewRegex ();
	QListView const * getListViewRegex () const;
    QComboBox * getFilterColorRegex ();
    QComboBox const * getFilterColorRegex () const;
	QListView * getListViewColorRegex ();
	QListView const * getListViewColorRegex () const;
	QListView * getListViewTID ();
	QListView const * getListViewTID () const;
	void setLevel (int i);
	int getLevel () const;
	bool scopesEnabled () const;
	bool autoScrollEnabled () const;
	bool reuseTabEnabled () const;
	bool filterEnabled () const;
	bool buffEnabled () const;
	bool clrFltEnabled () const;
	E_FilterMode fltMode () const;
	void changeEvent (QEvent* e);
	void dropEvent (QDropEvent * event);
	void dragEnterEvent (QDragEnterEvent *event);
	bool eventFilter (QObject * o, QEvent * e);
	void recompileRegexps ();
	void recompileColorRegexps ();

public slots:
	void onHotkeyShowOrHide ();

private slots:
	void loadState ();
	void loadPresets ();
	void storeState ();
	void storePresets ();
	void timerHit ();
	void onQuit ();
	void onEditFind ();
	void onEditFindNext ();
	void onEditFindPrev ();
	void onFileLoad ();
	void openFiles (QStringList const & list);
	void onFileSave ();
	void onFileExportToCSV ();
	void onColumnSetup ();
	void onFileFilterSetup ();
	void closeEvent (QCloseEvent *event);
	void iconActivated (QSystemTrayIcon::ActivationReason reason);
	void onQSearchEditingFinished ();
	void onSaveCurrentFileFilter ();
	void onPresetActivate (int idx);
	void onRegexActivate (int idx);
	void onRegexAdd ();
	void onRegexRm ();
	void onColorRegexActivate (int idx);
	void onColorRegexAdd ();
	void onColorRegexRm ();
	void onShowHelp ();
	void onFilterModeActivate (int idx);

private:
	void showServerStatus ();
	void setupMenuBar ();
	void createActions ();
	void createTrayIcon ();

	Ui::MainWindow * ui;
	Ui::SettingsDialog * m_settings;
	Ui::HelpDialog * m_help;
	bool m_hidden;
	bool m_was_maximized;
	QList<QString> m_app_names;					/// registered applications
	QList<columns_setup_t> m_columns_setup;		/// column setup for each registered application
	QList<columns_sizes_t> m_columns_sizes;		/// column sizes for each registered application
	QList<QColor> m_thread_colors;				/// predefined coloring of threads
	QList<QString> m_preset_names;				/// registered presets
	filter_presets_t m_filter_presets;			/// list of strings for each preset
	filter_regexs_t m_filter_regexs;			/// filtering regexps
	QList<QRegExp> m_regexps;
	std::vector<bool> m_regex_user_states;
	QString m_last_search;
	QTimer * m_timer;
	Server * m_server;
	QAction * m_minimize_action;
	QAction * m_maximize_action;
	QAction * m_restore_action;
	QAction * m_quit_action;
	QMenu * m_tray_menu;
	QSystemTrayIcon * m_tray_icon;
	QStandardItemModel * m_list_view_regex_model;
};

#endif // MAINWINDOW_H
