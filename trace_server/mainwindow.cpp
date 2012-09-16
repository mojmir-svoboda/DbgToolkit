#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settings.h"
#include "ui_help.h"
#include "modelview.h"
#include "server.h"
#include "connection.h"
#include <QTime>
#include <QTableView>
#include <QListView>
#include <QShortcut>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QMetaType>
#include <QVariant>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QItemDelegate>
#include <QStandardItemModel>
#include <QUrl>
#include <QtPlugin>
#include "settings.h"
#include "utils.h"
#include "../tlv_parser/tlv_parser.h"
#include "help.h"
#include "version.h"
#include "serialization.h"
#include "constants.h"
#include "dock.h"
#include "utils_qstandarditem.h"

#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

#if (defined WIN32) && (defined STATIC)
	Q_IMPORT_PLUGIN(qico);
//	Q_IMPORT_PLUGIN(qsvg); //@TODO: NEZAPOMENOUT ODKOMENTOVAT!
#endif

void MainWindow::loadNetworkSettings ()
{
	QSettings settings("MojoMir", "TraceServer");
	m_config.m_trace_addr = settings.value("trace_addr", "127.0.0.1").toString();
	m_config.m_trace_port = settings.value("trace_port", Server::default_port).toInt();
	m_config.m_profiler_addr = settings.value("profiler_addr", "127.0.0.1").toString();
	m_config.m_profiler_port = settings.value("profiler_port", 13147).toInt();
}

MainWindow::MainWindow (QWidget * parent, bool quit_delay, bool dump_mode)
	: QMainWindow(parent)
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
	, m_dock_mgr()
	, m_plots_dock(0)
	, m_plot_tree_view(0)
{
	//QDir::setSearchPaths("icons", QStringList(QDir::currentPath()));
	ui->setupUi(this);
	ui->tabTrace->setTabsClosable(true);

	m_settings_dialog = new QDialog(this);
	m_settings_dialog->setWindowFlags(Qt::Sheet);
	ui_settings = new Ui::SettingsDialog();
	ui_settings->setupUi(m_settings_dialog);

	m_plot_tree_view = new TreeView(this);
	m_plot_tree_view->setHidingLinearParents(false);
	m_plots_dock = m_dock_mgr.mkDockWidget(this, m_plot_tree_view, QString("plot list"), Qt::LeftDockWidgetArea);
	restoreDockWidget(m_plots_dock);
	m_plots_dock->setVisible(false);

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
	bool const on_top = settings.value("onTopCheckBox", true).toBool();
	if (on_top)
	{
		onOnTop(on_top);
	}
	ui_settings->onTopCheckBox->setChecked(on_top);

	loadNetworkSettings();
	m_server = new Server(m_config.m_trace_addr, m_config.m_trace_port, this, quit_delay);
	showServerStatus();
	connect(ui->qSearchComboBox->lineEdit(), SIGNAL(editingFinished()), this, SLOT(onQSearchEditingFinished()));
	connect(ui->qSearchComboBox, SIGNAL(activated(QString const &)), this, SLOT(onQSearchEditingFinished()));
	connect(ui->qFilterLineEdit, SIGNAL(returnPressed()), this, SLOT(onQFilterLineEditFinished()));

	size_t const n = tlv::get_tag_count();
	QString msg_tag;
	for (size_t i = tlv::tag_time; i < n; ++i)
	{
		char const * name = tlv::get_tag_name(i);
		if (name)
		{
			QString qname = QString::fromStdString(name);
			if (i == tlv::tag_msg)
				msg_tag = qname;
			ui->qSearchColumnComboBox->addItem(qname);
		}
	}
	ui->qSearchColumnComboBox->addItem("trace_server");
	ui->qSearchColumnComboBox->setCurrentIndex(ui->qSearchColumnComboBox->findText(msg_tag));

	m_timer->setInterval(5000);
	connect(m_timer, SIGNAL(timeout()) , this, SLOT(timerHit()));
	m_timer->start();
	setupMenuBar();

	getWidgetFile()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetFile(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtFileTree(QModelIndex)));
	connect(getWidgetFile(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtFileTree(QModelIndex)));
	getWidgetFile()->header()->hide();

	getWidgetCtx()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetCtx(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtCtxTree(QModelIndex)));
	connect(getWidgetCtx(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtCtxTree(QModelIndex)));
	getWidgetCtx()->header()->hide();

	getWidgetTID()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetTID(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtTIDList(QModelIndex)));
	connect(getWidgetTID(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtTIDList(QModelIndex)));

	connect(getTabTrace(), SIGNAL(currentChanged(int)), m_server, SLOT(onTabTraceFocus(int)));

	getWidgetLvl()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetLvl(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtLvlList(QModelIndex)));
	connect(getWidgetLvl(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtLvlList(QModelIndex)));
	getWidgetLvl()->header()->hide();

	connect(ui->levelSpinBox, SIGNAL(valueChanged(int)), m_server, SLOT(onLevelValueChanged(int)));
	connect(ui->filterFileCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onFilterFile(int)));
	connect(ui->plotsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onPlotStateChanged(int)));
	connect(ui->plotSaveAllButton, SIGNAL(clicked()), this, SLOT(onPlotSaveAllButton()));
	connect(ui->plotsToolButton, SIGNAL(clicked()), this, SLOT(onPlotsToolButton()));
	connect(m_plots_dock, SIGNAL(dockClosed()), this, SLOT(onPlotsClosed()));

	connect(ui->buffCheckBox, SIGNAL(stateChanged(int)), m_server, SLOT(onBufferingStateChanged(int)));
	
	connect(ui_settings->onTopCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onOnTop(int)));//@FIXME: this has some issues
	connect(ui_settings->reuseTabCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onReuseTabChanged(int)));
	//connect(ui->clrFiltersCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onClrFiltersStateChanged(int)));
	connect(ui->presetComboBox, SIGNAL(activated(int)), this, SLOT(onPresetActivate(int)));
	connect(ui->presetAddButton, SIGNAL(clicked()), this, SLOT(onAddCurrentFileFilter()));
	connect(ui->presetRmButton, SIGNAL(clicked()), this, SLOT(onRmCurrentFileFilter()));
	connect(ui->presetSaveButton, SIGNAL(clicked()), this, SLOT(onSaveCurrentFileFilter()));
	connect(ui->presetResetButton, SIGNAL(clicked()), m_server, SLOT(onClearCurrentFileFilter()));

	getWidgetRegex()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	getWidgetRegex()->header()->hide();
	connect(getWidgetTID(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtTIDList(QModelIndex)));
	connect(getWidgetRegex(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtRegexList(QModelIndex)));
	connect(getWidgetRegex(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtRegexList(QModelIndex)));
	connect(ui->comboBoxRegex, SIGNAL(activated(int)), this, SLOT(onRegexActivate(int)));
	connect(ui->buttonAddRegex, SIGNAL(clicked()), this, SLOT(onRegexAdd()));
	connect(ui->buttonRmRegex, SIGNAL(clicked()), this, SLOT(onRegexRm()));

	getWidgetColorRegex()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetColorRegex(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtColorRegexList(QModelIndex)));
	connect(getWidgetColorRegex(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtColorRegexList(QModelIndex)));
	connect(ui->comboBoxColorRegex, SIGNAL(activated(int)), this, SLOT(onColorRegexActivate(int)));
	connect(ui->buttonAddColorRegex, SIGNAL(clicked()), this, SLOT(onColorRegexAdd()));
	connect(ui->buttonRmColorRegex, SIGNAL(clicked()), this, SLOT(onColorRegexRm()));

	if (m_config.m_dump_mode)
	{
		ui->filterFileCheckBox->setEnabled(false);
		ui->autoScrollCheckBox->setEnabled(false);
		char const reason[] = "disabled due to -d option";
		ui->autoScrollCheckBox->setToolTip(tr(reason));
		ui->filterFileCheckBox->setToolTip(tr(reason));
	}
	else
	{
		ui->autoScrollCheckBox->setToolTip(tr("auto scrolls to bottom if checked"));
		ui->filterFileCheckBox->setToolTip(tr("enables filtering via filter tabs"));
	}
	//ui_settings->reuseTabCheckBox->setToolTip(tr("reuses compatible tab instead of creating new one"));
	//ui_settings->clrFiltersCheckBox->setToolTip(tr("force clearing of filters when reuseTab is checked"));
	//ui_settings->scopesCheckBox->setToolTip(tr("hides scopes if checked"));
	ui_settings->onTopCheckBox->setToolTip(tr("keeps window on top if checked. have to restart program, sorry"));
	ui->buffCheckBox->setToolTip(tr("turns on/off buffering of messages on client side."));
	ui->presetComboBox->setToolTip(tr("selects and applies saved preset file filter"));
	ui->presetAddButton->setToolTip(tr("saves current filter state as new preset"));
	ui->presetRmButton->setToolTip(tr("removes currently selected preset"));
	ui->presetSaveButton->setToolTip(tr("saves current filter state as currently selected preset"));
	ui->presetResetButton->setToolTip(tr("clear current filter"));
	ui->levelSpinBox->setToolTip(tr("adjusts debug level of client side"));
	ui->qSearchComboBox->setToolTip(tr("search text in logged text"));
	ui->qFilterLineEdit->setToolTip(tr("quick inclusive filter: adds string to regex filter as regex .*string.*"));
	ui->qSearchColumnComboBox->setToolTip(tr("specifies column to search"));

	m_status_label = new QLabel(m_server->getStatus());
	QString human_version(g_Version);
	human_version.chop(human_version.lastIndexOf(QChar('-')));
	QLabel * version_label = new QLabel(tr("Ver: %1").arg(human_version));
	statusBar()->addPermanentWidget(version_label);
	statusBar()->addWidget(m_status_label);

	connect(ui->tabTrace, SIGNAL(tabCloseRequested(int)), m_server, SLOT(onCloseTabWithIndex(int)));
	QTimer::singleShot(0, this, SLOT(loadState()));	// trigger lazy load of settings
	setWindowTitle("trace_server");
}

MainWindow::~MainWindow()
{
#ifdef WIN32
	UnregisterHotKey(winId(), 0);
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
	m_config.m_hidden = true;
	QMainWindow::hide();
}

void MainWindow::showNormal ()
{
	m_config.m_hidden = false;
	QMainWindow::showNormal();
}

void MainWindow::showMaximized ()
{
	m_config.m_hidden = false;
	QMainWindow::showMaximized();
}

void MainWindow::createActions ()
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

QTabWidget * MainWindow::getTabTrace () { return ui->tabTrace; }
QTabWidget const * MainWindow::getTabTrace () const { return ui->tabTrace; }
TreeView * MainWindow::getWidgetFile () { return ui->treeViewFile; }
TreeView const * MainWindow::getWidgetFile () const { return ui->treeViewFile; }
QTreeView * MainWindow::getWidgetCtx () { return ui->treeViewCtx; }
QTreeView const * MainWindow::getWidgetCtx () const { return ui->treeViewCtx; }
QComboBox * MainWindow::getFilterRegex () { return ui->comboBoxRegex; }
QComboBox const * MainWindow::getFilterRegex () const { return ui->comboBoxRegex; }
QTreeView * MainWindow::getWidgetRegex () { return ui->treeViewRegex; }
QTreeView const * MainWindow::getWidgetRegex () const { return ui->treeViewRegex; }
QComboBox * MainWindow::getFilterColorRegex () { return ui->comboBoxColorRegex; }
QComboBox const * MainWindow::getFilterColorRegex () const { return ui->comboBoxColorRegex; }
QListView * MainWindow::getWidgetColorRegex () { return ui->listViewColorRegex; }
QListView const * MainWindow::getWidgetColorRegex () const { return ui->listViewColorRegex; }
QListView * MainWindow::getWidgetTID () { return ui->listViewTID; }
QListView const * MainWindow::getWidgetTID () const { return ui->listViewTID; }
QTreeView * MainWindow::getWidgetLvl () { return ui->treeViewLvl; }
QTreeView const * MainWindow::getWidgetLvl () const { return ui->treeViewLvl; }

QTreeView const * MainWindow::getWidgetPlots () const { return m_plot_tree_view; }

bool MainWindow::scopesEnabled () const { return ui_settings->scopesCheckBox->isChecked(); }
bool MainWindow::indentEnabled () const { return ui_settings->indentCheckBox->isChecked(); }
int MainWindow::indentLevel () const { return ui_settings->indentSpinBox->value(); }
bool MainWindow::cutPathEnabled () const { return ui_settings->cutPathCheckBox->isChecked(); }
int MainWindow::cutPathLevel () const { return ui_settings->cutPathSpinBox->value(); }
bool MainWindow::cutNamespaceEnabled () const { return ui_settings->cutNamespaceCheckBox->isChecked(); }
int MainWindow::cutNamespaceLevel () const { return ui_settings->cutNamespaceSpinBox->value(); }
bool MainWindow::onTopEnabled () const { return ui_settings->onTopCheckBox->isChecked(); }
bool MainWindow::filterEnabled () const { return ui->filterFileCheckBox->isEnabled() && ui->filterFileCheckBox->isChecked(); }
bool MainWindow::plotEnabled () const { return ui->plotsCheckBox->isChecked(); }
bool MainWindow::reuseTabEnabled () const { return ui_settings->reuseTabCheckBox->isChecked(); }
bool MainWindow::autoScrollEnabled () const { return !m_config.m_dump_mode && ui->autoScrollCheckBox->isChecked(); }
bool MainWindow::buffEnabled () const { return ui->buffCheckBox->isChecked(); }
Qt::CheckState MainWindow::buffState () const { return ui->buffCheckBox->checkState(); }
bool MainWindow::clrFltEnabled () const { return ui_settings->clrFiltersCheckBox->isChecked(); }
bool MainWindow::statsEnabled () const { return ui_settings->traceStatsCheckBox->isChecked(); }

bool MainWindow::filterPaneVertical () const
{
	return ui_settings->filterPaneComboBox->currentText() == QString("Vertical");
}

void MainWindow::setLevel (int i)
{
	bool const old = ui->levelSpinBox->blockSignals(true);
	ui->levelSpinBox->setValue(i);
	ui->levelSpinBox->blockSignals(old);
}

int MainWindow::getLevel () const
{
	int const current = ui->levelSpinBox->value();
	return current;
}

void MainWindow::onQuit ()
{
	m_tray_icon->setVisible(false);
	m_tray_icon->hide();
	storeState();
	qApp->quit();
}

void MainWindow::onReuseTabChanged (int state)
{
	ui_settings->clrFiltersCheckBox->setEnabled(state);
}

void MainWindow::onPlotStateChanged (int state)
{
	m_server->onHidePlots();
	if (state == Qt::Checked)
	{
		Connection * conn = m_server->findCurrentConnection();
		if (!conn) return;

		conn->onShowPlots();
	}
}

void MainWindow::onPlotsToolButton ()
{
	if (ui->plotsToolButton->isChecked())
	{
		m_plots_dock->show();
		restoreDockWidget(m_plots_dock);

		if (Connection * conn = m_server->findCurrentConnection())
		{
			m_plot_tree_view->setModel(conn->m_plots_model);
			connect(m_plot_tree_view, SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtPlotTree(QModelIndex)));
		}
	}
	else
	{
		m_plots_dock->hide();
	}
}

void MainWindow::onPlotsClosed ()
{
	ui->plotsToolButton->setChecked(false);
}

void MainWindow::onFilterFile (int state)
{
	ui->presetComboBox->setEnabled(state);
	m_server->onFilterFile(state);
}


void MainWindow::onEditFind ()
{
	int const tab_idx = getTabTrace()->currentIndex();
	if (tab_idx < 0)
		return;

	bool ok;
	QString search = QInputDialog::getText(this, tr("Find"), tr("Text:"), QLineEdit::Normal, m_config.m_last_search, &ok);
	if (ok)
	{
		m_config.m_last_search = search;
		if (int const pos = ui->qSearchComboBox->findText(search) >= 0)
			ui->qSearchComboBox->setCurrentIndex(pos);
		else
		{
			ui->qSearchComboBox->addItem(search);
			ui->qSearchComboBox->setCurrentIndex(ui->qSearchComboBox->findText(search));
		}
		onQSearch(search);
	}
}

void MainWindow::onQSearch (QString const & text)
{
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	appendToSearchHistory(text);
	QString qcolumn = ui->qSearchColumnComboBox->currentText();
	qDebug("onQSearch: col=%s text=%s", qcolumn.toStdString().c_str(), text.toStdString().c_str());
	bool const search_all = (qcolumn == ".*");
	if (search_all)
	{
		conn->findText(text);
	}
	else
	{
		size_t const tag_idx = tlv::tag_for_name(qcolumn.toStdString().c_str());
		if (tag_idx != tlv::tag_invalid)
		{
			conn->findText(text, tag_idx);
		}
	}
}

void MainWindow::onQSearchEditingFinished ()
{
	if (!getTabTrace()->currentWidget()) return;

	QString text = ui->qSearchComboBox->currentText();
	onQSearch(text);
}

void MainWindow::onQFilterLineEditFinished ()
{
	if (!getTabTrace()->currentWidget()) return;

	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	if (ui->qFilterLineEdit->text().size() == 0)
		return;

	QString text(".*");
	text.append(ui->qFilterLineEdit->text());
	text.append(".*");
	conn->appendToRegexFilters(text.toStdString(), true, true); // @TODO: second true can use global incl/excl fmode

	QStandardItemModel * model = static_cast<QStandardItemModel *>(getWidgetRegex()->model());
	QStandardItem * root = model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, text);
	if (!child)
	{
		QList<QStandardItem *> row_items = addTriRow(text, Qt::Checked, true);
		root->appendRow(row_items);
		child = findChildByText(root, text);
		child->setCheckState(Qt::Checked);
	}

	for (int i = 0, ie = ui->tabFilters->count(); i < ie; ++i)
	{
		if (ui->tabFilters->tabText(i) == "RegExp")
		{
			ui->tabFilters->setCurrentIndex(i);
			break;
		}
	}

	conn->recompileRegexps();
	conn->onInvalidateFilter();
}

void MainWindow::appendToSearchHistory (QString const & str)
{
	if (str.length() == 0)
		return;
	m_config.m_search_history.insert(str);
	m_config.saveSearchHistory();
	updateSearchHistory();
	ui->qSearchComboBox->setCurrentIndex(ui->qSearchComboBox->findText(str));
}

void MainWindow::updateSearchHistory ()
{
	ui->qSearchComboBox->clear();
	for (size_t i = 0, ie = m_config.m_search_history.size(); i < ie; ++i)
		ui->qSearchComboBox->addItem(m_config.m_search_history[i]);
}

void MainWindow::onEditFindNext ()
{
	if (!getTabTrace()->currentWidget()) return;
	if (Connection * conn = m_server->findCurrentConnection())
		conn->findNext();
}

void MainWindow::onEditFindPrev ()
{ }

void MainWindow::openFiles (QStringList const & files)
{
	for (size_t i = 0, ie = files.size(); i < ie; ++i)
	{
		QString fname = files.at(i);
		if (fname != "")
		{
			QFile file(fname);
			if (!file.open(QIODevice::ReadOnly))
			{
				QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
				return;
			}
			QDataStream in(&file);
			m_server->incomingDataStream(in);
			file.close();
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
		m_server->copyStorageTo(filename);
	}
}

void MainWindow::onFileExportToCSV ()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Trace Files (*.csv)"));

	if (filename != "")
	{
		m_server->exportStorageToCSV(filename);
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

void MainWindow::onDumpFilters ()
{
	QDialog dialog(this);
	dialog.setWindowFlags(Qt::Sheet);
	m_help->setupUi(&dialog);
	m_help->helpTextEdit->clear();

	QString text(tr("Dumping current filters:\n"));
	QString session_string;

	if (Connection * conn = m_server->findCurrentConnection())
	{
		SessionState const & ss = conn->sessionState();
		std::string ff;
		ss.m_file_filters.dump_filter(ff);

		QString cols;
		for (int i = 0, ie = ss.m_colorized_texts.size(); i < ie; ++i)
		{
			ColorizedText const & x = ss.m_colorized_texts.at(i);
			cols += QString("%1 %2 %3\n")
						.arg(QString::fromStdString(x.m_regex_str))
						.arg(x.m_qcolor.name())
						.arg(x.m_is_enabled ? " on" : "off");
		}
		QString regs;
		for (int i = 0, ie = ss.m_filtered_regexps.size(); i < ie; ++i)
		{
			FilteredRegex const & x = ss.m_filtered_regexps.at(i);
			regs += QString("%1 %2 %3\n")
						.arg(QString::fromStdString(x.m_regex_str))
						.arg(x.m_is_inclusive ? "incl" : "excl")
						.arg(x.m_is_enabled ? " on" : "off");
		}
		QString lvls;
		for (int i = 0, ie = ss.m_lvl_filters.size(); i < ie; ++i)
		{
			FilteredLevel const & x = ss.m_lvl_filters.at(i);
			lvls += QString("%1 %2 %3\n")
						.arg(x.m_level_str)
						.arg(x.m_is_enabled ? " on" : "off")
						.arg(x.m_state ? "forced" : "");
		}
		QString ctxs;
		for (int i = 0, ie = ss.m_ctx_filters.size(); i < ie; ++i)
		{
			FilteredContext const & x = ss.m_ctx_filters.at(i);
			ctxs += QString("%1 %2 %3\n")
						.arg(x.m_ctx_str)
						.arg(x.m_state)
						.arg(x.m_is_enabled ? " on" : "off");
		}

		session_string = QString("Files:\n%1\n\nColors:\n%2\n\nRegExps:\n%3\n\nLevels:\n%4\n\nContexts:\n%5\n\n")
				.arg(ff.c_str())
				.arg(cols)
				.arg(regs)
				.arg(lvls)
				.arg(ctxs);
	}

	m_help->helpTextEdit->setPlainText(text + session_string);
	m_help->helpTextEdit->setReadOnly(true);
	dialog.exec();
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

void MainWindow::onPresetActivate (QString const & pname)
{
	onPresetActivate(ui->presetComboBox->findText(pname));
}

void MainWindow::onPresetActivate (Connection * conn, QString const & pname)
{
	if (!conn) return;

	SessionState dummy;
	if (loadSession(dummy, pname))
	{
		conn->destroyModelFile();
		std::swap(conn->m_session_state.m_file_filters.root, dummy.m_file_filters.root);
		conn->setupModelFile();
		conn->m_session_state.m_filtered_regexps = dummy.m_filtered_regexps;
		conn->m_session_state.m_colorized_texts = dummy.m_colorized_texts;
		conn->m_session_state.m_lvl_filters = dummy.m_lvl_filters;
		conn->m_session_state.m_ctx_filters = dummy.m_ctx_filters;
		//@TODO: this blows under linux, i wonder why?
		//conn->m_session_state.m_filtered_regexps.swap(dummy.m_filtered_regexps);
		//conn->m_session_state.m_colorized_texts.swap(dummy.m_colorized_texts);

		getWidgetFile()->hideLinearParents();
		getWidgetFile()->syncExpandState();

		conn->onInvalidateFilter();
		setPresetNameIntoComboBox(pname);
	}
	else
	{
		ui->presetComboBox->removeItem(ui->presetComboBox->findText(pname));
		m_config.m_preset_names.removeAll(pname);
		storePresetNames();
	}
}

void MainWindow::onPresetActivate (int idx)
{
	if (idx == -1) return;
	if (Connection * conn = m_server->findCurrentConnection())
	{
		conn->onClearCurrentFileFilter();
		onPresetActivate(conn, m_config.m_preset_names.at(idx));
	}
}

void MainWindow::onRegexActivate (int idx)
{
	if (idx == -1) return;
	if (!getTabTrace()->currentWidget()) return;

	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	onRegexAdd();
}

void MainWindow::onRegexAdd ()
{
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QString qItem = ui->comboBoxRegex->currentText();
	QStandardItem * root = static_cast<QStandardItemModel *>(getWidgetRegex()->model())->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addTriRow(qItem, Qt::Unchecked, true);
		root->appendRow(row_items);
		conn->appendToRegexFilters(qItem.toStdString(), false, true);
		conn->recompileRegexps();
	}
}

void MainWindow::onRegexRm ()
{
	QStandardItemModel * model = static_cast<QStandardItemModel *>(getWidgetRegex()->model());
	QModelIndex const idx = getWidgetRegex()->currentIndex();
	QStandardItem * item = model->itemFromIndex(idx);
	if (!item)
		return;
	QString const & val = model->data(idx, Qt::DisplayRole).toString();
	model->removeRow(idx.row());
	Connection * conn = m_server->findCurrentConnection();
	if (conn)
	{
		conn->removeFromRegexFilters(val.toStdString());
		conn->recompileRegexps();
	}
}

// @TODO: all the color stuff is almost duplicate, remove duplicity
void MainWindow::onColorRegexActivate (int idx)
{
	if (idx == -1) return;
	if (!getTabTrace()->currentWidget()) return;

	onColorRegexAdd();
}

void MainWindow::onColorRegexAdd ()
{
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QString qItem = ui->comboBoxColorRegex->currentText();
	if (!qItem.length())
		return;
	QStandardItem * root = static_cast<QStandardItemModel *>(getWidgetColorRegex()->model())->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(qItem, false);
		root->appendRow(row_items);

		conn->appendToColorRegexFilters(qItem.toStdString());
	}
	conn->recompileColorRegexps();
}

void MainWindow::onColorRegexRm ()
{
	Connection * conn = m_server->findCurrentConnection();
	QStandardItemModel * model = static_cast<QStandardItemModel *>(getWidgetColorRegex()->model());
	QModelIndex const idx = getWidgetColorRegex()->currentIndex();
	QStandardItem * item = model->itemFromIndex(idx);
	if (!item)
		return;
	QString const & val = model->data(idx, Qt::DisplayRole).toString();
	model->removeRow(idx.row());

	if (conn)
	{
		conn->removeFromColorRegexFilters(val.toStdString());
		conn->recompileColorRegexps();
	}
}

void MainWindow::onSaveCurrentFileFilter ()
{
	QString txt = ui->presetComboBox->currentText();
	if (0 == txt.size())
		if (Connection * conn = m_server->findCurrentConnection())
			txt = getPresetPath(conn->sessionState().getAppName(), g_defaultPresetName);
	onSaveCurrentFileFilterTo(txt);
}

void MainWindow::onSaveCurrentFileFilterTo (QString const & preset_name)
{
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	if (!preset_name.isEmpty())
	{
		int idx = findPresetName(preset_name);
		if (idx == -1)
			idx = addPresetName(preset_name);

		qDebug("new preset_name[%i]=%s", idx, preset_name.toStdString().c_str());
		saveCurrentSession(preset_name);

		ui->presetComboBox->clear();
		for (size_t i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
			ui->presetComboBox->addItem(m_config.m_preset_names.at(i));
		setPresetNameIntoComboBox(preset_name);
		storePresetNames();
	}
}

void MainWindow::setPresetNameIntoComboBox (QString const & pname)
{
	ui->presetComboBox->setCurrentIndex(ui->presetComboBox->findText(pname));
}

void MainWindow::onAddCurrentFileFilter ()
{
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QString const filled_text = getPresetPath(conn->sessionState().getAppName(), g_defaultPresetName);
	QStringList items(m_config.m_preset_names);
	items.push_front(filled_text);

	bool ok = false;
	QString preset_name = QInputDialog::getItem(this, tr("Save current preset"), tr("Preset name:"), items, 0, true, &ok);
	if (ok && !preset_name.isEmpty())
	{
		onSaveCurrentFileFilterTo(preset_name);
	}
}

void MainWindow::onRmCurrentFileFilter ()
{
	QString const preset_name = ui->presetComboBox->currentText();
	int idx = findPresetName(preset_name);
	if (idx == -1)
		return;

	qDebug("removing preset_name[%i]=%s", idx, preset_name.toStdString().c_str());
	
	QString fname = getPresetFileName(m_config.m_appdir, preset_name);
	qDebug("confirm to remove session file=%s", fname.toStdString().c_str());

	QMessageBox msg_box;
	QPushButton * b_del = msg_box.addButton(tr("Yes, Delete"), QMessageBox::ActionRole);
	QPushButton * b_abort = msg_box.addButton(QMessageBox::Abort);
	msg_box.exec();
	if (msg_box.clickedButton() == b_abort)
		return;

	QFile qf(fname);
	qf.remove();

	ui->presetComboBox->clear();

	m_config.m_preset_names.erase(std::remove(m_config.m_preset_names.begin(), m_config.m_preset_names.end(), preset_name), m_config.m_preset_names.end());
	for (size_t i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
		ui->presetComboBox->addItem(m_config.m_preset_names.at(i));
	//setPresetNameIntoComboBox(preset_name);
	storePresetNames();
}

void MainWindow::onPlotSaveAllButton ()
{
	QSettings settings("MojoMir", "TraceServer");
	settings.setValue("MainWindow/State", saveState());
	settings.setValue("MainWindow/Geometry", saveGeometry());
	
}

void MainWindow::onPlotRestoreButton ()
{
	QSettings settings("MojoMir", "TraceServer");
	restoreState(settings.value("MainWindow/State").toByteArray());
	restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
}

void MainWindow::storePresetNames ()
{
	QSettings settings("MojoMir", "TraceServer");
	write_list_of_strings(settings, "known-presets", "preset", m_config.m_preset_names);
}

void MainWindow::onGotoFileFilter () { ui->tabFilters->setCurrentIndex(0); }
void MainWindow::onGotoLevelFilter () { ui->tabFilters->setCurrentIndex(1); }
void MainWindow::onGotoColorFilter () { ui->tabFilters->setCurrentIndex(5); }
void MainWindow::onGotoRegexFilter () { ui->tabFilters->setCurrentIndex(4); }

void MainWindow::setupMenuBar ()
{
	// File
	QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(tr("File &Load..."), this, SLOT(onFileLoad()), QKeySequence(Qt::ControlModifier + Qt::Key_O));
	fileMenu->addAction(tr("File &Save..."), this, SLOT(onFileSave()), QKeySequence(Qt::ControlModifier + Qt::Key_S));
	fileMenu->addAction(tr("File &Save As CSV format"), this, SLOT(onFileExportToCSV()), QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_S));
	fileMenu->addSeparator();
	fileMenu->addAction(tr("Quit program"), this, SLOT(onQuit()), QKeySequence::Quit);

	// Edit
	QMenu * editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(tr("Find"), this, SLOT(onEditFind()), QKeySequence::Find);
	editMenu->addAction(tr("Find Next"), this, SLOT(onEditFindNext()), QKeySequence::FindNext);
	editMenu->addAction(tr("Find Prev"), this, SLOT(onEditFindPrev()), QKeySequence::FindPrevious);
	new QShortcut(QKeySequence(Qt::Key_Slash), this, SLOT(onEditFind()));
	editMenu->addSeparator();
	editMenu->addAction(tr("Copy"), m_server, SLOT(onCopyToClipboard()), QKeySequence::Copy);
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Insert), this, SLOT(onCopyToClipboard()));
	editMenu->addSeparator();
	editMenu->addAction(tr("Close Tab"), m_server, SLOT(onCloseCurrentTab()), QKeySequence(Qt::ControlModifier + Qt::Key_W));

	// Filter
	QMenu * filterMenu = menuBar()->addMenu(tr("Fi&lter"));
	filterMenu->addAction(tr("Hide previous rows"), m_server, SLOT(onHidePrevFromRow()), QKeySequence(Qt::Key_Delete));
	filterMenu->addAction(tr("Unhide previous rows"), m_server, SLOT(onUnhidePrevFromRow()), QKeySequence(Qt::ControlModifier + Qt::Key_Delete));
	filterMenu->addAction(tr("Toggle reference row"), m_server, SLOT(onToggleRefFromRow()), QKeySequence(Qt::Key_Space));
	filterMenu->addAction(tr("Exclude file:line row"), m_server, SLOT(onExcludeFileLine()), QKeySequence(Qt::Key_X));
	filterMenu->addAction(tr("Goto file filter"), this, SLOT(onGotoFileFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F1));
	filterMenu->addAction(tr("Goto level filter"), this, SLOT(onGotoLevelFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F2));
	filterMenu->addAction(tr("Goto regex filter"), this, SLOT(onGotoRegexFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F3));
	filterMenu->addAction(tr("Goto color filter"), this, SLOT(onGotoColorFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F4));
		
	// Clear
	QMenu * clearMenu = menuBar()->addMenu(tr("&Clear"));
	clearMenu->addAction(tr("Clear current table view"), m_server, SLOT(onClearCurrentView()), QKeySequence(Qt::Key_C));
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_L), this, SLOT(onClearCurrentView()));
	clearMenu->addAction(tr("Clear current file filter"), m_server, SLOT(onClearCurrentFileFilter()));
	clearMenu->addAction(tr("Clear current context filter"), m_server, SLOT(onClearCurrentCtxFilter()));
	clearMenu->addAction(tr("Clear current thread id filter"), m_server, SLOT(onClearCurrentCtxFilter()));
	clearMenu->addAction(tr("Clear current colorized regexp filter"), m_server, SLOT(onClearCurrentColorizedRegexFilter()));
	clearMenu->addAction(tr("Clear current regexp filter"), m_server, SLOT(onClearCurrentRegexFilter()));
	clearMenu->addAction(tr("Clear current collapsed scope filter"), m_server, SLOT(onClearCurrentScopeFilter()));

	// Tools
	QMenu * tools = menuBar()->addMenu(tr("&Settings"));
	tools->addAction(tr("&Options"), this, SLOT(onSetupAction()), QKeySequence(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_O));
	//tools->addAction(tr("Save Current Filter As..."), this, SLOT(onSaveCurrentFileFilter()));
	tools->addSeparator();
	tools->addAction(tr("Save options now (this will NOT save presets)"), this, SLOT(storeState()), QKeySequence(Qt::AltModifier + Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_O));

	// Help
	QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(tr("Help"), this, SLOT(onShowHelp()));
	helpMenu->addAction(tr("Dump filters"), this, SLOT(onDumpFilters()));
}

void MainWindow::saveSession (SessionState const & s, QString const & preset_name) const
{
	QString fname = getPresetFileName(m_config.m_appdir, preset_name);
	qDebug("store file=%s", fname.toStdString().c_str());
	saveSessionState(s, fname.toAscii());
}

void MainWindow::storePresets ()
{
	qDebug("storePresets()");
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	storePresetNames();

	for (size_t i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
		if (!m_config.m_preset_names.at(i).isEmpty())
			saveSession(conn->sessionState(), m_config.m_preset_names.at(i));
}

void MainWindow::saveCurrentSession (QString const & preset_name)
{
	qDebug("MainWindow::saveCurrentSession(), name=%s", preset_name.toStdString().c_str());
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QSettings settings("MojoMir", "TraceServer");

	saveSession(conn->sessionState(), preset_name);
}

bool MainWindow::loadSession (SessionState & s, QString const & preset_name)
{
	QString fname = getPresetFileName(m_config.m_appdir, preset_name);
	qDebug("load file=%s", fname.toStdString().c_str());
	s.m_file_filters.clear();
	return loadSessionState(s, fname.toAscii());
}

void MainWindow::loadPresets ()
{
	m_config.m_preset_names.clear();

	QSettings settings("MojoMir", "TraceServer");
	read_list_of_strings(settings, "known-presets", "preset", m_config.m_preset_names);
	for (size_t i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
	{
		ui->presetComboBox->addItem(m_config.m_preset_names.at(i));
	}

	// @NOTE: this is only for smooth transition only
	for (size_t i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
	{
		qDebug("reading preset: %s", m_config.m_preset_names.at(i).toStdString().c_str());
		QString const prs_name = tr("preset_%1").arg(m_config.m_preset_names[i]);
		QStringList const split = prs_name.split("/");

		qDebug("split[0]: %s", split.at(0).toStdString().c_str());
		qDebug("split[1]: %s", split.at(1).toStdString().c_str());
		if (settings.childGroups().contains(split.at(0)))
		{
			settings.beginGroup(split.at(0));
			if (settings.childGroups().contains(split.at(1)))
			{
				settings.beginGroup(split.at(1));
				typedef QList<QString>			filter_regexs_t;
				typedef QList<QString>			filter_preset_t;

				filter_preset_t m_file_filters;
				filter_preset_t m_colortext_regexs;
				filter_preset_t m_colortext_colors;
				filter_preset_t m_colortext_enabled;
				filter_preset_t m_regex_filters;
				filter_preset_t m_regex_fmode;
				filter_preset_t m_regex_enabled;

				read_list_of_strings(settings, "items", "item", m_file_filters);
				read_list_of_strings(settings, "cregexps", "item", m_colortext_regexs);
				read_list_of_strings(settings, "cregexps_colors", "item", m_colortext_colors);
				read_list_of_strings(settings, "cregexps_enabled", "item", m_colortext_enabled);
				read_list_of_strings(settings, "regexps", "item", m_regex_filters);
				read_list_of_strings(settings, "regexps_fmode", "item", m_regex_fmode);
				read_list_of_strings(settings, "regexps_enabled", "item", m_regex_enabled);

				SessionState ss;
				for (int f = 0, fe = m_file_filters.size(); f < fe; ++f)
					ss.m_file_filters.set_to_state(m_file_filters.at(f).toStdString(), e_Checked);

				saveSession(ss, m_config.m_preset_names.at(i));

				settings.remove("");
				settings.endGroup();
			}
			settings.endGroup();
		}
		else
		{
			// @TODO
			// check if on disk
			// and if not, clear name from combobox
		}
	}
}

void MainWindow::storeState ()
{
	QSettings settings("MojoMir", "TraceServer");

	settings.setValue("trace_addr", m_config.m_trace_addr);
	settings.setValue("trace_port", m_config.m_trace_port);
	settings.setValue("profiler_addr", m_config.m_profiler_addr);
	settings.setValue("profiler_port", m_config.m_profiler_port);

	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
	settings.setValue("splitter", ui->splitter->saveState());
	settings.setValue("autoScrollCheckBox", ui->autoScrollCheckBox->isChecked());
	settings.setValue("filterFileCheckBox", ui->filterFileCheckBox->isChecked());
	settings.setValue("buffCheckBox", ui->buffCheckBox->isChecked());
	settings.setValue("clrFiltersCheckBox", ui_settings->clrFiltersCheckBox->isChecked());
	//settings.setValue("filterModeComboBox", ui->filterModeComboBox->currentIndex());
	settings.setValue("filterPaneComboBox", ui_settings->filterPaneComboBox->currentIndex());
	settings.setValue("levelSpinBox", ui->levelSpinBox->value());

	settings.setValue("trace_stats", ui_settings->traceStatsCheckBox->isChecked());
	settings.setValue("reuseTabCheckBox", ui_settings->reuseTabCheckBox->isChecked());
	settings.setValue("scopesCheckBox1", ui_settings->scopesCheckBox->isChecked());
	settings.setValue("indentCheckBox", ui_settings->indentCheckBox->isChecked());
	settings.setValue("cutPathCheckBox", ui_settings->cutPathCheckBox->isChecked());
	settings.setValue("cutNamespaceCheckBox", ui_settings->cutNamespaceCheckBox->isChecked());
	settings.setValue("indentSpinBox", ui_settings->indentSpinBox->value());
	settings.setValue("cutPathSpinBox", ui_settings->cutPathSpinBox->value());
	settings.setValue("cutNamespaceSpinBox", ui_settings->cutNamespaceSpinBox->value());
	settings.setValue("onTopCheckBox", ui_settings->onTopCheckBox->isChecked());

	write_list_of_strings(settings, "known-applications", "application", m_config.m_app_names);
	for (size_t i = 0, ie = m_config.m_app_names.size(); i < ie; ++i)
	{
		settings.beginGroup(tr("column_order_%1").arg(m_config.m_app_names[i]));
		{
			write_list_of_strings(settings, "orders", "column", m_config.m_columns_setup.at(i));
		}
		settings.endGroup();

		settings.beginGroup(tr("column_sizes_%1").arg(m_config.m_app_names[i]));
		{
			QList<columns_sizes_t>::const_iterator oi = m_config.m_columns_sizes.constBegin();
			while (oi != m_config.m_columns_sizes.constEnd())
			{
				settings.beginWriteArray("sizes");
				int const size = (*oi).size();
				for (int ai = 0; ai < size; ++ai) {
					settings.setArrayIndex(ai);
					settings.setValue("column", QString::number((*oi).at(ai)));
				}
				settings.endArray();
				++oi;
			}
		}
		settings.endGroup();

		settings.beginGroup(tr("column_align_%1").arg(m_config.m_app_names[i]));
		{
			write_list_of_strings(settings, "aligns", "column", m_config.m_columns_align.at(i));
		}
		settings.endGroup();

		settings.beginGroup(tr("column_elide_%1").arg(m_config.m_app_names[i]));
		{
			write_list_of_strings(settings, "elides", "column", m_config.m_columns_elide.at(i));
		}
		settings.endGroup();
	}

#ifdef WIN32
	settings.setValue("hotkeyCode", m_config.m_hotkey);
#endif
}

void MainWindow::loadState ()
{
	m_config.m_app_names.clear();
	m_config.m_columns_setup.clear();
	m_config.m_columns_sizes.clear();
	m_config.loadSearchHistory();
	updateSearchHistory();

	QSettings settings("MojoMir", "TraceServer");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	int const pane_val = settings.value("filterPaneComboBox", 0).toInt();
	ui_settings->filterPaneComboBox->setCurrentIndex(pane_val);
	if (settings.contains("splitter"))
	{
		ui->splitter->restoreState(settings.value("splitter").toByteArray());
		ui->splitter->setOrientation(pane_val ? Qt::Vertical : Qt::Horizontal);
	}

	ui_settings->traceStatsCheckBox->setChecked(settings.value("trace_stats", true).toBool());

	ui->autoScrollCheckBox->setChecked(settings.value("autoScrollCheckBox", true).toBool());
	ui_settings->reuseTabCheckBox->setChecked(settings.value("reuseTabCheckBox", true).toBool());
	ui_settings->scopesCheckBox->setChecked(settings.value("scopesCheckBox1", true).toBool());
	ui_settings->indentCheckBox->setChecked(settings.value("indentCheckBox", true).toBool());
	ui_settings->cutPathCheckBox->setChecked(settings.value("cutPathCheckBox", true).toBool());
	ui_settings->cutNamespaceCheckBox->setChecked(settings.value("cutNamespaceCheckBox", true).toBool());

	ui_settings->indentSpinBox->setValue(settings.value("indentSpinBox", 2).toInt());
	ui_settings->cutPathSpinBox->setValue(settings.value("cutPathSpinBox", 1).toInt());
	ui_settings->cutNamespaceSpinBox->setValue(settings.value("cutNamespaceSpinBox", 1).toInt());

	ui->filterFileCheckBox->setChecked(settings.value("filterFileCheckBox", true).toBool());
	ui->buffCheckBox->setChecked(settings.value("buffCheckBox", true).toBool());
	ui_settings->clrFiltersCheckBox->setChecked(settings.value("clrFiltersCheckBox", false).toBool());
	//ui->filterModeComboBox->setCurrentIndex(settings.value("filterModeComboBox").toInt());
	//@TODO: delete filterMode from registry if exists
	ui->levelSpinBox->setValue(settings.value("levelSpinBox", 3).toInt());

	read_list_of_strings(settings, "known-applications", "application", m_config.m_app_names);
	for (size_t i = 0, ie = m_config.m_app_names.size(); i < ie; ++i)
	{
		m_config.m_columns_setup.push_back(columns_setup_t());
		settings.beginGroup(tr("column_order_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "orders", "column", m_config.m_columns_setup.back());
		}
		settings.endGroup();
		
		m_config.m_columns_sizes.push_back(columns_sizes_t());
		settings.beginGroup(tr("column_sizes_%1").arg(m_config.m_app_names[i]));
		{
			int const size = settings.beginReadArray("sizes");
			for (int i = 0; i < size; ++i) {
				settings.setArrayIndex(i);
				m_config.m_columns_sizes.back().push_back(settings.value("column").toInt());
			}
			settings.endArray();
		}
		settings.endGroup();

		m_config.m_columns_align.push_back(columns_align_t());
		settings.beginGroup(tr("column_align_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "aligns", "column", m_config.m_columns_align.back());
		}
		settings.endGroup();

		if (m_config.m_columns_align.back().size() < m_config.m_columns_sizes.back().size())
			for (int i = 0, ie = m_config.m_columns_sizes.back().size(); i < ie; ++i)
				m_config.m_columns_align.back().push_back(QString("L"));

		m_config.m_columns_elide.push_back(columns_elide_t());
		settings.beginGroup(tr("column_elide_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "elides", "column", m_config.m_columns_elide.back());
		}
		settings.endGroup();

		if (m_config.m_columns_elide.back().size() < m_config.m_columns_sizes.back().size())
			for (int i = 0, ie = m_config.m_columns_sizes.back().size(); i < ie; ++i)
				m_config.m_columns_elide.back().push_back(QString("R"));
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
	UnregisterHotKey(winId(), 0);
	RegisterHotKey(winId(), 0, mod, LOBYTE(hotkey));
#endif

	loadPresets();
	getWidgetFile()->setEnabled(filterEnabled());
	ui->plotsToolButton->setChecked(m_plots_dock->isVisible());


	qApp->installEventFilter(this);
}


void MainWindow::iconActivated (QSystemTrayIcon::ActivationReason reason)
{
	switch (reason) {
		case QSystemTrayIcon::Trigger:     break;
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
	if (e->type() == QEvent::Shortcut)
	{
		QShortcutEvent * se = static_cast<QShortcutEvent *>(e);
		if (se->key() == QKeySequence(Qt::ControlModifier + Qt::Key_Insert))
		{
			m_server->onCopyToClipboard();
			return true;
		}
	}
	return false;
}

