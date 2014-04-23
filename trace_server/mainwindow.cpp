#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settings.h"
#include "ui_help.h"
#include "server.h"
#include "connection.h"
#include <QTime>
#include <QTableView>
#include <QListView>
#include <QShortcut>
#include <QInputDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QSettings>
#include <QMetaType>
#include <QMimeData>
#include <QVariant>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QItemDelegate>
#include <QStandardItemModel>
#include <QUrl>
#include <QClipboard>
#include <QTimer>
#include <tlv_parser/tlv_parser.h>
#include "help.h"
#include "version.h"
#include "constants.h"
#include "dock.h"
#include <ui_controlbarcommon.h>
#include "utils.h"
#include "utils_qstandarditem.h"
#include "utils_qsettings.h"
#include "utils_history.h"
#include "qt_plugins.h"

void MainWindow::loadNetworkSettings ()
{
	QSettings settings("MojoMir", "TraceServer");
	m_config.m_trace_addr = settings.value("trace_addr", "127.0.0.1").toString();
	m_config.m_trace_port = settings.value("trace_port", Server::default_port).toInt();
	m_config.m_profiler_addr = settings.value("profiler_addr", "127.0.0.1").toString();
	m_config.m_profiler_port = settings.value("profiler_port", 13147).toInt();
}

MainWindow::MainWindow (QWidget * parent, bool quit_delay, bool dump_mode, QString const & log_name, int level)
	: QMainWindow(parent)
	, m_time_units(0.001f)
	, ui(new Ui::MainWindow)
	, ui_settings(0)
	, m_help(new Ui::HelpDialog)
	, m_timer(new QTimer(this))
	, m_server(0)
	, m_minimize_action(0)
	, m_maximize_action(0)
	, m_restore_action(0)
	, m_quit_action(0)
	, m_tray_menu(0)
	, m_tray_icon(0)
	, m_settings_dialog(0)
	, m_dock_mgr(this, QStringList(QString(g_traceServerName)))
	, m_docked_name(g_traceServerName)
	, m_log_name(log_name)
	, m_start_level(level)
{
	qDebug("================================================================================");
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	ui->setupUi(this);

	m_settings_dialog = new QDialog(this);
	m_settings_dialog->setWindowFlags(Qt::Sheet);
	ui_settings = new Ui::SettingsDialog();
	ui_settings->setupUi(m_settings_dialog);

	QString const homedir = QDir::homePath();
	m_config.m_appdir = homedir + "/.flogging";
	m_config.m_dump_mode = dump_mode;

	// tray stuff
	createActions();
	createTrayIcon();
	QIcon icon(":images/Icon1.ico");
	setWindowIcon(icon);
	m_tray_icon->setVisible(true);
	m_tray_icon->show();

	setAcceptDrops(true);
	setDockNestingEnabled(true);
	setAnimated(false);

	QSettings settings("MojoMir", "TraceServer");
	bool const on_top = settings.value("onTopCheckBox", false).toBool();
	if (on_top)
	{
		onOnTop(on_top);
	}

	loadNetworkSettings();
	m_server = new Server(m_config.m_trace_addr, m_config.m_trace_port, this, quit_delay);
	connect(m_server, SIGNAL(newConnection(Connection *)), this, SLOT(newConnection(Connection *)));
	showServerStatus();

	m_timer->setInterval(5000);
	connect(m_timer, SIGNAL(timeout()) , this, SLOT(timerHit()));
	m_timer->start();
	setupMenuBar();

	connect(ui->dockManagerButton, SIGNAL(clicked()), this, SLOT(onDockManagerButton()));
	connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSave()));
	connect(ui->saveAsButton, SIGNAL(clicked()), this, SLOT(onSaveAs()));

	connect(m_dock_mgr.controlUI()->levelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onLevelValueChanged(int)));
	connect(m_dock_mgr.controlUI()->buffCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onBufferingStateChanged(int)));
	connect(m_dock_mgr.controlUI()->presetComboBox, SIGNAL(activated(int)), this, SLOT(onPresetChanged(int)));
	connect(m_dock_mgr.controlUI()->activatePresetButton, SIGNAL(clicked()), this, SLOT(onPresetActivate()));
	connect(m_dock_mgr.controlUI()->presetSaveButton, SIGNAL(clicked()), this, SLOT(onSaveCurrentState()));
	connect(m_dock_mgr.controlUI()->presetAddButton, SIGNAL(clicked()), this, SLOT(onAddPreset()));
	connect(m_dock_mgr.controlUI()->presetRmButton, SIGNAL(clicked()), this, SLOT(onRmCurrentPreset()));
	connect(m_dock_mgr.controlUI()->plotSlider, SIGNAL(valueChanged(int)), this, SLOT(onPlotStateChanged(int)));
	connect(m_dock_mgr.controlUI()->tableSlider, SIGNAL(valueChanged(int)), this, SLOT(onTablesStateChanged(int)));
	//connect(ui->presetResetButton, SIGNAL(clicked()), this, SLOT(onClearCurrentState()));

	//connect(qApp, SIGNAL(void focusChanged(QWidget *, QWidget *)), this, SLOT(void onFocusChanged(QWidget *, QWidget *)));

	/// status bar
	m_status_label = new QLabel(m_server->getStatus());
	QString human_version(g_Version);
	human_version.chop(human_version.lastIndexOf(QChar('-')));
	QLabel * version_label = new QLabel(tr("Ver: %1").arg(human_version));
	statusBar()->addPermanentWidget(version_label);
	statusBar()->addWidget(m_status_label);

	QTimer::singleShot(0, this, SLOT(loadState()));	// trigger lazy load of settings
	setWindowTitle(g_traceServerName);
	setObjectName(g_traceServerName);
}

MainWindow::~MainWindow()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
#ifdef WIN32
	UnregisterHotKey(getHWNDForWidget(this), 0);
#endif
	m_server->setParent(0);
	delete m_server;
	delete m_tray_icon;
	delete m_tray_menu;
	delete m_help;
	delete ui;
	delete ui_settings;
}

void MainWindow::hide ()
{
	qDebug("%s", __FUNCTION__);
	m_config.m_hidden = true;
	QMainWindow::hide();
}

void MainWindow::showNormal ()
{
	qDebug("%s", __FUNCTION__);
	m_config.m_hidden = false;
	QMainWindow::showNormal();
}

void MainWindow::showMaximized ()
{
	qDebug("%s", __FUNCTION__);
	m_config.m_hidden = false;
	QMainWindow::showMaximized();
}

void MainWindow::createActions ()
{
	qDebug("%s", __FUNCTION__);
	m_minimize_action = new QAction(tr("Mi&nimize"), this);
	connect(m_minimize_action, SIGNAL(triggered()), this, SLOT(hide()));

	m_maximize_action = new QAction(tr("Ma&ximize"), this);
	connect(m_maximize_action, SIGNAL(triggered()), this, SLOT(showMaximized()));

	m_restore_action = new QAction(tr("&Restore"), this);
	connect(m_restore_action, SIGNAL(triggered()), this, SLOT(showNormal()));

	m_quit_action = new QAction(tr("&Quit"), this);
	connect(m_quit_action, SIGNAL(triggered()), this, SLOT(onQuit()));
}

void MainWindow::createTrayIcon ()
{
	qDebug("%s", __FUNCTION__);
	m_tray_menu = new QMenu(this);
	m_tray_menu->addAction(m_minimize_action);
	m_tray_menu->addAction(m_maximize_action);
	m_tray_menu->addAction(m_restore_action);
	m_tray_menu->addSeparator();
	m_tray_menu->addAction(m_quit_action);

	QIcon icon(":/images/Icon1.ico");
	m_tray_icon = new QSystemTrayIcon(icon, this);
	m_tray_icon->setContextMenu(m_tray_menu);

	connect(m_tray_icon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
	connect(m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::dropEvent (QDropEvent * event)
{
	QMimeData const * mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		QStringList pathList;
		QList<QUrl> urlList = mimeData->urls();

		for (int i = 0; i < urlList.size() && i < 32; ++i)
			pathList.append(urlList.at(i).toLocalFile());
		openFiles(pathList);
	}

	event->acceptProposedAction();
}

void MainWindow::dragEnterEvent (QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void MainWindow::showServerStatus ()
{
	statusBar()->showMessage(m_server->getStatus());
}

void MainWindow::timerHit ()
{
	showServerStatus();
}

//QTreeView const * MainWindow::getDockedWidgetsTreeView () const { return m_docked_widgets_tree_view; }

bool MainWindow::onTopEnabled () const { return ui_settings->onTopCheckBox->isChecked(); }
int MainWindow::plotState () const { return m_dock_mgr.controlUI()->plotSlider->value(); }
int MainWindow::tableState () const { return m_dock_mgr.controlUI()->tableSlider->value(); }
int MainWindow::ganttState () const { return m_dock_mgr.controlUI()->ganttSlider->value(); }

bool MainWindow::buffEnabled () const { return m_dock_mgr.controlUI()->buffCheckBox->isChecked(); }
Qt::CheckState MainWindow::buffState () const { return m_dock_mgr.controlUI()->buffCheckBox->checkState(); }

void MainWindow::setLevel (int i)
{
	bool const old = m_dock_mgr.controlUI()->levelSpinBox->blockSignals(true);
	m_dock_mgr.controlUI()->levelSpinBox->setValue(i);
	m_dock_mgr.controlUI()->levelSpinBox->blockSignals(old);
}
int MainWindow::getLevel () const
{
	int const current = m_dock_mgr.controlUI()->levelSpinBox->value();
	return current;
}

void MainWindow::onQuit ()
{
	qDebug("onQuit: hide systray, store state, qApp->quit");

	m_tray_icon->setVisible(false);
	m_tray_icon->hide();
	storeState();

	while (!m_connections.empty())
	{
		Connection * c = m_connections.back();
		m_connections.pop_back();
		destroyConnection(c);
	}

	QTimer::singleShot(0, this, SLOT(onQuitReally()));	// trigger lazy quit
}

void MainWindow::onQuitReally ()
{
	qDebug("%s", __FUNCTION__);
	qApp->quit();
}

void MainWindow::onDockManagerButton ()
{
	if (ui->dockManagerButton->isChecked())
	{
		m_dock_mgr.m_docked_widgets->show();
		m_dock_mgr.m_config.m_show = true;
		restoreDockWidget(m_dock_mgr.m_docked_widgets);
	}
	else
	{
		m_dock_mgr.m_docked_widgets->hide();
		m_dock_mgr.m_config.m_show = false;
	}
}

void MainWindow::onDockManagerClosed ()
{
	ui->dockManagerButton->setChecked(false);
}

void MainWindow::onPlotStateChanged (int state)
{
}

void MainWindow::onTablesStateChanged (int state)
{
}

void MainWindow::tailFiles (QStringList const & files)
{
	for (int i = 0, ie = files.size(); i < ie; ++i)
	{
		QString fname = files.at(i);
		if (fname != "")
			createTailDataStream(fname);
	}
}

void MainWindow::onFileTail ()
{
	QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("*.*"));
	QStringList files;
	files << fname;
	tailFiles(files);
}

void MainWindow::onLogTail ()
{
	//setupSeparatorChar("|");
	createTailLogStream(m_log_name, "|");
}

void MainWindow::openFiles (QStringList const & files)
{
	for (int i = 0, ie = files.size(); i < ie; ++i)
	{
		QString fname = files.at(i);
		if (fname != "")
		{
			importDataStream(fname);
		}
	}
}

void MainWindow::onFileLoad ()
{
	// @TODO: multi-selection?
	QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("TLV Trace Files (*.tlv_trace)"));
	QStringList files;
	files << fname;
	openFiles(files);
}

void MainWindow::onFileSave ()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Trace Files (*.tlv_trace)"));

	if (filename != "")
	{
		copyStorageTo(filename);
	}
}

void MainWindow::onFileExportToCSV ()
{
/*	QString filename = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Trace Files (*.csv)"));

	if (filename != "")
	{
    if (Connection * conn = findCurrentConnection())
      conn->exportStorageToCSV(filename);
	}*/
}

void MainWindow::onOnTop (int const state)
{
	if (state == 0)
	{
		Qt::WindowFlags const old = windowFlags();
		Qt::WindowFlags const newflags = Qt::Window | (old & ~(Qt::WindowStaysOnTopHint));
		setWindowFlags(newflags);
		show();
	}
	else
	{
		//setWindowFlags(Qt::WindowStaysOnTopHint); //@NOTE: win users lacks the min and max buttons. sigh.
		setWindowFlags(Qt::WindowStaysOnTopHint);
		show();
	}
}

void MainWindow::onHotkeyShowOrHide ()
{
	bool const not_on_top = !isActiveWindow();
	qDebug("onHotkeyShowOrHide() isActive=%u", not_on_top);
	m_config.m_hidden = !m_config.m_hidden;
		
	if (m_config.m_hidden)
	{
		m_config.m_was_maximized = isMaximized();
		hide();
	}
	else
	{
		if (m_config.m_was_maximized)
			showMaximized();
		else
			showNormal();
		raise();
		activateWindow();
	}
}


void MainWindow::onShowHelp ()
{
	QDialog dialog(this);
	dialog.setWindowFlags(Qt::Sheet);
	m_help->setupUi(&dialog);
	m_help->helpTextEdit->clear();
	m_help->helpTextEdit->setHtml(QString(html_help));
	m_help->helpTextEdit->setReadOnly(true);
	dialog.exec();
}

void MainWindow::storeGeometry ()
{
	qDebug("%s", __FUNCTION__);
	QSettings settings("MojoMir", "TraceServer");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
}

void MainWindow::onDockRestoreButton ()
{
	qDebug("%s", __FUNCTION__);
	QSettings settings("MojoMir", "TraceServer");
	restoreState(settings.value("windowState").toByteArray());
	restoreGeometry(settings.value("geometry").toByteArray());
}

void MainWindow::setupMenuBar ()
{
	qDebug("%s", __FUNCTION__);
	// File
	QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(tr("File &Load..."), this, SLOT(onFileLoad()), QKeySequence(Qt::ControlModifier + Qt::Key_O));
	fileMenu->addAction(tr("File &Tail..."), this, SLOT(onFileTail()), QKeySequence(Qt::ControlModifier + Qt::Key_T));
	fileMenu->addAction(tr("Trace Server Log"), this, SLOT(onLogTail()), QKeySequence(Qt::ControlModifier + Qt::AltModifier + Qt::Key_L));
	fileMenu->addAction(tr("File &Save..."), this, SLOT(onFileSave()), QKeySequence(Qt::ControlModifier + Qt::Key_S));
	fileMenu->addAction(tr("File &Save As CSV format"), this, SLOT(onFileExportToCSV()), QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_S));
	fileMenu->addSeparator();
	fileMenu->addAction(tr("Quit program"), this, SLOT(onQuit()), QKeySequence::Quit);

	// Edit
	//QMenu * editMenu = menuBar()->addMenu(tr("&Edit"));
/*
	editMenu->addAction(tr("Find"), this, SLOT(onFind()), QKeySequence::Find);
	//editMenu->addAction(tr("Find"), this, SLOT(onFind()),	Qt::ControlModifier + Qt::Key_F);
	editMenu->addAction(tr("Find"), this, SLOT(onFindAllRefs()), Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_F);
	editMenu->addAction(tr("Find Next"), this, SLOT(onFindNext()), QKeySequence::FindNext);
	editMenu->addAction(tr("Find Prev"), this, SLOT(onFindPrev()), QKeySequence::FindPrevious);
	new QShortcut(QKeySequence(Qt::Key_Slash), this, SLOT(onFind()));
*/
	//editMenu->addAction(tr("Find and Select All"), this, SLOT(onFindAllButton()));
	//editMenu->addAction(tr("Goto Next Tag or Selection"), this, SLOT(onNextToView()));

	//editMenu->addSeparator();
	//editMenu->addAction(tr("Close Tab"), this, SLOT(onCloseCurrentTab()), QKeySequence(Qt::ControlModifier + Qt::Key_W));

	// Filter
	//QMenu * filterMenu = menuBar()->addMenu(tr("Fi&lter"));
	//filterMenu->addAction(tr("Goto file filter"), this, SLOT(onGotoFileFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F1));
	//filterMenu->addAction(tr("Goto level filter"), this, SLOT(onGotoLevelFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F2));
	//filterMenu->addAction(tr("Goto regex filter"), this, SLOT(onGotoRegexFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F3));
	//filterMenu->addAction(tr("Goto color filter"), this, SLOT(onGotoColorFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F4));

	// Tools
	//QMenu * tools = menuBar()->addMenu(tr("&Settings"));
	//tools->addAction(tr("&Options"), this, SLOT(onSetupAction()), QKeySequence(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_O));
	//tools->addAction(tr("Save Current Filter As..."), this, SLOT(onSaveCurrentFileFilter()));
	//tools->addSeparator();
	//tools->addAction(tr("Save options now (this will NOT save presets)"), this, SLOT(storeState()), QKeySequence(Qt::AltModifier + Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_O));

	// Help
	//QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
	//helpMenu->addAction(tr("Help"), this, SLOT(onShowHelp()));
	//helpMenu->addAction(tr("Dump filters"), this, SLOT(onDumpFilters()));

	//new QShortcut(QKeySequence(Qt::AltModifier + Qt::Key_Space), this, SLOT(onAutoScrollHotkey()));
}

void MainWindow::storeState ()
{
	qDebug("%s", __FUNCTION__);
	QSettings settings("MojoMir", "TraceServer");

	settings.setValue("trace_addr", m_config.m_trace_addr);
	settings.setValue("trace_port", m_config.m_trace_port);
	settings.setValue("profiler_addr", m_config.m_profiler_addr);
	settings.setValue("profiler_port", m_config.m_profiler_port);

	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());

	settings.setValue("tableSlider", m_dock_mgr.controlUI()->tableSlider->value());
	settings.setValue("plotSlider", m_dock_mgr.controlUI()->plotSlider->value());
	settings.setValue("ganttSlider", m_dock_mgr.controlUI()->ganttSlider->value());
	settings.setValue("buffCheckBox", m_dock_mgr.controlUI()->buffCheckBox->isChecked());
	settings.setValue("levelSpinBox", m_dock_mgr.controlUI()->levelSpinBox->value());

	//OBSOLETTEsettings.setValue("trace_stats", ui_settings->traceStatsCheckBox->isChecked());
	settings.setValue("onTopCheckBox", ui_settings->onTopCheckBox->isChecked());
	settings.setValue("presetComboBox", m_dock_mgr.controlUI()->presetComboBox->currentText());
	
	m_dock_mgr.saveConfig(m_config.m_appdir);
#ifdef WIN32
	settings.setValue("hotkeyCode", m_config.m_hotkey);
#endif
}

void MainWindow::restoreDockedWidgetGeometry ()
{
	QSettings settings("MojoMir", "TraceServer");

	//restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::loadState ()
{
	qDebug("%s", __FUNCTION__);
	m_config.loadHistory();

	QSettings settings("MojoMir", "TraceServer");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	int const pane_val = settings.value("filterPaneComboBox", 0).toInt();

	ui_settings->reuseTabCheckBox->setChecked(settings.value("reuseTabCheckBox", true).toBool());

	m_dock_mgr.controlUI()->tableSlider->setValue(settings.value("tableSlider", 0).toInt());
	m_dock_mgr.controlUI()->plotSlider->setValue(settings.value("plotSlider", 0).toInt());
	m_dock_mgr.controlUI()->ganttSlider->setValue(settings.value("ganttSlider", 0).toInt());
	m_dock_mgr.controlUI()->buffCheckBox->setChecked(settings.value("buffCheckBox", true).toBool());

	//@TODO: delete filterMode from registry if exists
	if (m_start_level == -1)
	{
		qDebug("reading saved level from cfg");
		m_dock_mgr.controlUI()->levelSpinBox->setValue(settings.value("levelSpinBox", 3).toInt());
	}
	else
	{
		qDebug("reading level from command line");
		m_dock_mgr.controlUI()->levelSpinBox->setValue(m_start_level);
	}

	if (m_config.m_thread_colors.empty())
	{
		for (size_t i = Qt::white; i < Qt::transparent; ++i)
			m_config.m_thread_colors.push_back(QColor(static_cast<Qt::GlobalColor>(i)));
	}

#ifdef WIN32
	unsigned const hotkeyCode = settings.value("hotkeyCode").toInt();
	m_config.m_hotkey = hotkeyCode ? hotkeyCode : VK_SCROLL;
	DWORD const hotkey = m_config.m_hotkey;
	int mod = 0;
	UnregisterHotKey(getHWNDForWidget(this), 0);
	RegisterHotKey(getHWNDForWidget(this), 0, mod, LOBYTE(hotkey));
#endif

	m_dock_mgr.loadConfig(m_config.m_appdir);
	m_dock_mgr.applyConfig();
	ui->dockManagerButton->setChecked(m_dock_mgr.m_config.m_show);
	qApp->installEventFilter(this);
}


void MainWindow::iconActivated (QSystemTrayIcon::ActivationReason reason)
{
	switch (reason) {
		case QSystemTrayIcon::Trigger:	   break;
		case QSystemTrayIcon::DoubleClick: onHotkeyShowOrHide(); break;
		case QSystemTrayIcon::MiddleClick: break;
		default: break;
	}
}

void MainWindow::closeEvent (QCloseEvent * event)
{
	storeState();

	m_config.m_hidden = true;
	hide();
	event->ignore();
}

void MainWindow::changeEvent (QEvent * e) { QMainWindow::changeEvent(e); }
bool MainWindow::eventFilter (QObject * target, QEvent * e)
{
	/*if (e->type() == QEvent::Shortcut)
	{
		QShortcutEvent * se = static_cast<QShortcutEvent *>(e);
		if (se->key() == QKeySequence(Qt::ControlModifier + Qt::Key_Insert))
		{

			//onCopyToClipboard();
			return true;
		}
	}*/
	return false;
}

void MainWindow::keyPressEvent (QKeyEvent * e)
{
	if (e->type() == QKeyEvent::KeyPress)
	{
		/*if (e->matches(QKeySequence::Copy))
		{
			e->accept();
		}
		else
		if (e->key() == Qt::Key_Escape)
		{
			if (m_find_widget && m_find_widget->isVisible())
			{
				m_find_widget->onCancel();
				e->accept();
			}
		}*/
	}
	QMainWindow::keyPressEvent(e);
}


void MainWindow::addNewApplication (QString const & appname)
{
	m_config.m_app_names.push_back(appname);
}

int MainWindow::createAppName (QString const & appname, E_SrcProtocol const proto)
{
	addNewApplication(appname);
	int const app_idx = static_cast<int>(m_config.m_app_names.size()) - 1;
	return app_idx;
}

