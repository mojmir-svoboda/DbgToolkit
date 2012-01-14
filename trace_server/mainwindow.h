#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTreeView>
#include "server.h"

namespace Ui {
	class MainWindow;
	class SettingsDialog;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	QTabWidget * getTabTrace ();
	QTabWidget const * getTabTrace () const;

	typedef QList<QString> columns_setup_t;
	typedef QList<int> columns_sizes_t;
	columns_setup_t const & getColumnSetup (size_t i) const { return m_columns_setup.at(i); }
	columns_setup_t & getColumnSetup (size_t i) { return m_columns_setup[i]; }
	columns_sizes_t const & getColumnSizes (size_t i) const { return m_columns_sizes.at(i); }
	columns_sizes_t & getColumnSizes (size_t i) { return m_columns_sizes[i]; }
	size_t findAppName (QString const & appname)
	{
		for (size_t i = 0, ie = m_app_names.size(); i < ie; ++i)
			if (m_app_names[i] == appname)
				return i;
		
		m_app_names.push_back(appname);
		m_columns_setup.push_back(columns_setup_t());
		m_columns_setup.reserve(16);
		m_columns_sizes.push_back(columns_sizes_t());
		m_columns_sizes.reserve(16);
		return m_app_names.size() - 1;
	}
	QList<QColor> const & getThreadColors () const { return m_thread_colors; }
	QTreeView * getTreeViewFile ();
	QTreeView const * getTreeViewFile () const;
	QTreeView * getTreeViewFunc ();
	QTreeView const * getTreeViewFunc () const;
	bool scopesEnabled () const;
	bool persistentEnabled () const;
	void changeEvent (QEvent* e);
	void dropEvent (QDropEvent * event);
	void dragEnterEvent (QDragEnterEvent *event);
	//bool eventFilter (QObject * o, QEvent * e);

public slots:
	void onHotkeyShowOrHide ();

private slots:
	void loadState ();
	void storeState ();
	void timerHit ();
	void onEditFind ();
	void onEditFindNext ();
	void onEditFindPrev ();
	void onFileImport ();
	void openFiles (QStringList const & list);
	void onFileExport ();
	void onSettings ();
	void closeEvent (QCloseEvent *event);

private:
	void showServerStatus ();
	void setupMenuBar ();

	Ui::MainWindow * ui;
	Ui::SettingsDialog * m_settings;
	bool m_hidden;
	QList<QString> m_app_names;
	QList<columns_setup_t> m_columns_setup;
	QList<columns_sizes_t> m_columns_sizes;
	QList<QColor> m_thread_colors;
	QString m_last_search;
	QTimer * m_timer;
	Server * m_server;
};

#endif // MAINWINDOW_H
