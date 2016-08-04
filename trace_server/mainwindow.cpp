#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settings.h"
#include "ui_mixer.h"
#include "ui_help.h"
#include <server/server.h>
#include "connection.h"
#include <QTime>
#include <QShortcut>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QLabel>
#include <QDropEvent>
#include <QMimeData>
#include <QStatusBar>
#include <widgets/help.h>
#include <widgets/mixer.h>
#include <trace_version/trace_version.h>
#include "constants.h"
#include <dock/dockwidget.h>
#include <widgets/setupdialogcsv.h>
#include <widgets/controlbar/controlbardockmanager.h>
#include <ui_controlbardockmanager.h>
#include "ui_setupdialogcsv.h"
#include <ui_controlbarcommon.h>
#include <utils/utils.h>
#include <utils/utils_qstandarditem.h>
#include <utils/utils_qsettings.h>
#include <utils/utils_history.h>
#include "qt_plugins.h"
#include "platform.h"

MainWindow::MainWindow (QWidget * parent, QString const & iface, unsigned short port, bool quit_delay, bool dump_mode, QString const & log_name, int level)
	: QMainWindow(parent)
	, m_time_units(0.001f)
	, ui(new Ui::MainWindow)
	, ui_settings(nullptr)
	, m_help(new Ui::HelpDialog)
	, m_timer(new QTimer(this))
	, m_server(nullptr)
	, m_windows_menu(nullptr)
	, m_file_menu(nullptr)
	, m_before_action(nullptr)
	, m_minimize_action(nullptr)
	, m_maximize_action(nullptr)
	, m_restore_action(nullptr)
	, m_quit_action(nullptr)
	, m_tray_menu(nullptr)
	, m_dock_mgr_button(nullptr)
	, m_status_widget(nullptr)
	, m_tray_icon(nullptr)
	, m_settings_dialog(nullptr)
	, m_setup_dialog_csv(nullptr)
	, m_dock_mgr(this, QStringList(QString(g_traceServerName)))
	, m_docked_name(g_traceServerName)
	, m_log_name(log_name)
	, m_appdir(QDir::homePath() + "/" + g_traceServerDirName)
	, m_start_level(level)
{
	qDebug("================================================================================");
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	ui->setupUi(this);

	setStyleSheet( "QMainWindow::separator { background: rgb(150, 150, 150); width: 1px; height: 1px; }");
	qApp->setStyleSheet("QToolTip { color: #efc090; background-color: #000066; border: 1px solid #efc090; }");

	m_settings_dialog = new QDialog(this);
	m_settings_dialog->setWindowFlags(Qt::Sheet);
	ui_settings = new Ui::SettingsDialog();
	ui_settings->setupUi(m_settings_dialog);

	m_setup_dialog_csv = new SetupDialogCSV(this);
	m_setup_dialog_csv->ui->preView->setModel(new QStandardItemModel());
	m_setup_dialog_csv->ui->columnList->setModel(new QStandardItemModel());

	//m_mixer = new Mixer(nullptr, m_dock_mgr.controlUI()->mixerButton, sizeof(context_t) * CHAR_BIT, sizeof(level_t) * CHAR_BIT);
	//Ui::Mixer * ui_mixer = new Ui::Mixer();
	//m_mixer->setupMixer(m_config.m_mixer);

	m_config.m_appdir = m_appdir;
	m_config.m_dump_mode = dump_mode;
	m_config.m_trace_addr = iface;
	m_config.m_trace_port = port;
	loadConfig();
	m_config.loadHistory(m_appdir);
	setConfigValuesToUI(m_config);

	// tray stuff
	createTrayActions();
	createTrayIcon();
	QIcon icon(":images/Icon.ico");
	setWindowIcon(icon);
	m_tray_icon->setVisible(true);
	m_tray_icon->show();

	setAcceptDrops(true);
	setDockNestingEnabled(true);
	setAnimated(false);

	bool const on_top = m_config.m_on_top;
	if (on_top)
	{
		onOnTop(on_top);
	}

	m_server = new Server(m_config.m_trace_addr, m_config.m_trace_port, this, quit_delay);
	connect(m_server, SIGNAL(newConnection(Connection *)), this, SLOT(newConnection(Connection *)));
	connect(m_server, SIGNAL(statusChanged(QString const &)), this, SLOT(onStatusChanged(QString const &)));

 	m_timer->setInterval(5000);
 	connect(m_timer, SIGNAL(timeout()) , this, SLOT(onTimerHit()));
 	m_timer->start();

	/// status widget
	m_status_widget = new QLabel(m_server->getRunningStatus());
	m_status_widget->setAlignment(Qt::AlignHCenter);
	if (statusBar())
		statusBar()->hide();

	setupMenuBar();

	//connect(m_dock_mgr.controlUI()->levelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onLevelValueChanged(int)));
	connect(m_dock_mgr.controlUI()->buffCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onBufferingStateChanged(int)));
	connect(m_dock_mgr.controlUI()->clrDataButton, SIGNAL(clicked()), this, SLOT(onClearAllData()));
	connect(m_dock_mgr.controlUI()->presetComboBox, SIGNAL(activated(int)), this, SLOT(onPresetChanged(int)));
	connect(m_dock_mgr.controlUI()->presetComboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onPresetAdd()));
	connect(m_dock_mgr.controlUI()->activatePresetButton, SIGNAL(clicked()), this, SLOT(onPresetApply()));
	connect(m_dock_mgr.controlUI()->presetSaveButton, SIGNAL(clicked()), this, SLOT(onPresetSave()));
	connect(m_dock_mgr.controlUI()->presetAddButton, SIGNAL(clicked()), this, SLOT(onPresetAdd()));
	connect(m_dock_mgr.controlUI()->presetRmButton, SIGNAL(clicked()), this, SLOT(onPresetRm()));
	//connect(m_dock_mgr.controlUI()->presetResetButton, SIGNAL(clicked()), this, SLOT(onPresetReset()));

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

void MainWindow::createTrayActions ()
{
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
	m_tray_menu = new QMenu(this);
	m_tray_menu->addAction(m_minimize_action);
	m_tray_menu->addAction(m_maximize_action);
	m_tray_menu->addAction(m_restore_action);
	m_tray_menu->addSeparator();
	m_tray_menu->addAction(m_quit_action);

	QIcon icon(":/images/Icon.ico");
	m_tray_icon = new QSystemTrayIcon(icon, this);
	m_tray_icon->setContextMenu(m_tray_menu);

	//connect(m_tray_icon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
	connect(m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::registerHotKey ()
{
#ifdef WIN32
	DWORD const hotkey = m_config.m_hotkey;
	int mod = 0;
	UnregisterHotKey(getHWNDForWidget(this), 0);
	RegisterHotKey(getHWNDForWidget(this), 0, mod, LOBYTE(hotkey));
#endif
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

void MainWindow::onStatusChanged (QString const & status)
{
	m_status_widget->setText(status);
	m_timer->start(5000);
}

void MainWindow::showServerStatus ()
{
	m_status_widget->setText(m_server->getRunningStatus());
}

void MainWindow::onTimerHit ()
{
	showServerStatus();
}

void MainWindow::onQuit ()
{
	qDebug("onQuit: hide systray, store state, qApp->quit");

	m_server->stop();

	m_tray_icon->setVisible(false);
	m_tray_icon->hide();
	storeState();

	while (!m_connections.empty())
	{
		Connection * c = m_connections.back();
		m_connections.pop_back();
		delete c;
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
	if (m_dock_mgr_button->isChecked())
	{
		m_dock_mgr.show();
		m_dock_mgr.m_dockwidget->show();
		m_dock_mgr.m_config.m_show = true;
		restoreDockWidget(m_dock_mgr.m_dockwidget);
	}
	else
	{
		m_dock_mgr.hide();
		m_dock_mgr.m_dockwidget->hide();
		m_dock_mgr.m_config.m_show = false;
	}
}

void MainWindow::onDockManagerClosed ()
{
	m_dock_mgr_button->setChecked(false);
}

void MainWindow::onReloadFile ()
{
	for (size_t i = 0, ie = m_reload_fnames.size(); i < ie; ++i)
	{
		QString const fname = m_reload_fnames[i];
		if (!fname.isEmpty())
			createTailDataStream(fname);
	}
	m_reload_fnames.clear();
}

void MainWindow::tailFiles (QStringList const & files)
{
	for (int i = 0, ie = files.size(); i < ie; ++i)
	{
		QString const fname = files.at(i);
		if (!fname.isEmpty())
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

void MainWindow::onFileLoadCSV ()
{
	QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("CSV File(s) (*.%1)").arg(g_traceFileExtCSV));
	QStringList files;
	files << fname;
	tailFiles(files); // spunt, @TODO
}

void MainWindow::onLogTail ()
{
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
	QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Trace File(s) (*.%1)").arg(g_traceFileExtTLV));
	QStringList files;
	files << fname;
	openFiles(files);
}

void MainWindow::onSaveData ()
{
	QString const dir = QFileDialog::getExistingDirectory(this, tr("Save data to directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty())
		return;

	if (mkPath(dir))
	{
		for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
			(*it)->onSaveData(dir);
	}
}

void MainWindow::onExportDataToCSV ()
{
	QString const dir = QFileDialog::getExistingDirectory(this, tr("Export csv to directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty())
		return;

	if (mkPath(dir))
	{
		for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
			(*it)->onExportDataToCSV(dir);
	}
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

void MainWindow::onRecentFile ()
{
	QAction * action = qobject_cast<QAction *>(sender());
	if (action)
	{
		QString const fname = action->data().toString();
		QString const ext = QFileInfo(fname).suffix();

		bool const is_tlv = (0 == QString::compare(ext, g_traceFileExtTLV, Qt::CaseInsensitive));
		bool const is_csv = (0 == QString::compare(ext, g_traceFileExtCSV, Qt::CaseInsensitive));
		if (is_tlv)
			importDataStream(fname);
		else if (is_csv)
			createTailDataStream(fname);
		else
			createTailDataStream(fname);
	}
}

void MainWindow::setupMenuBar ()
{
	qDebug("%s", __FUNCTION__);
	m_dock_mgr_button = new QToolButton(0);
	m_dock_mgr_button->setObjectName(QStringLiteral("dockManagerButton"));
	m_dock_mgr_button->setMinimumSize(QSize(0, 0));
	//m_dock_mgr_button->setFont(font);
	m_dock_mgr_button->setCheckable(true);
	m_dock_mgr_button->setChecked(false);
	m_dock_mgr_button->setToolTip(QApplication::translate("MainWindow", "Central management of docked data widgets", 0));
	m_dock_mgr_button->setText(QApplication::translate("MainWindow", "Widget Manager", 0));
	menuBar()->setCornerWidget(m_dock_mgr_button, Qt::TopLeftCorner);
	connect(m_dock_mgr_button, SIGNAL(clicked()), this, SLOT(onDockManagerButton()));
	m_dock_mgr_button->setStyleSheet("\
		QToolButton  { border: 1px solid #8f8f91; border-radius: 4px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #e6e7fa, stop: 1 #dadbff); } \
		QToolButton:checked { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbdd, stop: 1 #d6d7da); }");

	menuBar()->setCornerWidget(m_status_widget, Qt::TopRightCorner);
	connect(m_status_widget, SIGNAL(clicked()), this, SLOT(onDockManagerButton()));

	// File
	m_file_menu = menuBar()->addMenu(tr("&File"));
	m_file_menu->addAction(tr("Data file &Load..."), this, SLOT(onFileLoad()), QKeySequence(Qt::ControlModifier + Qt::Key_O));
	m_file_menu->addAction(tr("&Save raw client data as..."), this, SLOT(onSaveData()));
	m_file_menu->addAction(tr("&Export data as CSV format..."), this, SLOT(onExportDataToCSV()));
	m_file_menu->addSeparator();
	m_file_menu->addAction(tr("Load &CSV file..."), this, SLOT(onFileLoadCSV()));
	m_file_menu->addAction(tr("&Follow text file..."), this, SLOT(onFileTail()), QKeySequence(Qt::ControlModifier + Qt::Key_T));

	m_file_menu->addSeparator();
	syncHistoryToRecent(m_config.m_recent_history);

	m_before_action = m_file_menu->addSeparator();
	m_file_menu->addAction(tr("Quit program"), this, SLOT(onQuit()), QKeySequence::Quit);

	// View
	QMenu * viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(tr("&Save preset"), this, SLOT(onSave()), QKeySequence::Save);
	viewMenu->addAction(tr("Save preset &as..."), this, SLOT(onSaveAs()), QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_S));
	viewMenu->addSeparator();

	// View
	QMenu * actionMenu = menuBar()->addMenu(tr("Actions"));
	QKeySequence all_deldata(Qt::CTRL + Qt::Key_A, Qt::CTRL + Qt::Key_D);
	actionMenu->addAction(tr("All widgets - Clear Data"), this, SLOT(onClearAllData()), all_deldata);
	QKeySequence all_colorlast(Qt::CTRL + Qt::Key_A, Qt::CTRL + Qt::Key_E);
	actionMenu->addAction(tr("All widgets - Color last line "), this, SLOT(onColorAllLastLine()), all_colorlast);
	QKeySequence all_scrolllast(Qt::CTRL + Qt::Key_A, Qt::CTRL + Qt::Key_Z);
	actionMenu->addAction(tr("All widgets - Scroll to last "), this, SLOT(onAllScrollToLast()), all_scrolllast);
	QKeySequence quick_string(Qt::CTRL + Qt::Key_Q, Qt::CTRL + Qt::Key_S);
	actionMenu->addAction(tr("Quick - String ! filter"), this, SLOT(onQuickNotString()), quick_string);

	// widget's tool dockable widgets.
	m_windows_menu = menuBar()->addMenu(tr("&Windows"));

	// Help
	QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addSeparator();
	helpMenu->addAction(tr("&Remove configuration files"), this, SLOT(onRemoveConfigurationFiles()));
	helpMenu->addAction(tr("Show server log"), this, SLOT(onLogTail()), QKeySequence(Qt::ControlModifier + Qt::AltModifier + Qt::Key_L));
	QAction * ver = helpMenu->addAction(tr("Version %1").arg(g_Version));
	ver->setEnabled(false);

	// Tools
	//QMenu * tools = menuBar()->addMenu(tr("&Settings"));
	//tools->addAction(tr("&Options"), this, SLOT(onSetupAction()), QKeySequence(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_O));
	//tools->addAction(tr("Save Current Filter As..."), this, SLOT(onSaveCurrentFileFilter()));
	//tools->addSeparator();
	//tools->addAction(tr("Save options now (this will NOT save presets)"), this, SLOT(storeState()), QKeySequence(Qt::AltModifier + Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_O));

	// Help
	//QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
	//helpMenu->addAction(tr("Help"), this, SLOT(onShowHelp()));
}

void MainWindow::addWindowAction (QAction * action)
{
	m_windows_menu->addAction(action);
}
void MainWindow::rmWindowAction (QAction * action)
{
	m_windows_menu->removeAction(action);
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
	/*if (e->type() == QEvent::Shortcut) {
		QShortcutEvent * se = static_cast<QShortcutEvent *>(e);
		if (se->key() == QKeySequence(Qt::ControlModifier + Qt::Key_Insert)) {
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
			e->accept();
		else if (e->key() == Qt::Key_Escape)
			if (m_find_widget && m_find_widget->isVisible()) {
				m_find_widget->onCancel();
				e->accept();
			}*/
	}
	QMainWindow::keyPressEvent(e);
}

void MainWindow::onRemoveConfigurationFiles ()
{
	//QString const path = m_appdir; // @NOTE: rather not.. will not risk deletion of other files than mine
	QString const path = QDir::homePath() + "/" + g_traceServerDirName;
	if (path.isEmpty())
		return;
	QMessageBox msg_box;
	QPushButton * b_del = msg_box.addButton(tr("Yes, Delete"), QMessageBox::ActionRole);
	QPushButton * b_abort = msg_box.addButton(QMessageBox::Abort);
	msg_box.exec();
	if (msg_box.clickedButton() == b_abort)
		return;

	QDir d_out(path);
	d_out.removeRecursively();

	QDir d_new;
	d_new.mkpath(path);
}


