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
	m_trace_addr = settings.value("trace_addr", "127.0.0.1").toString();
	m_trace_port = settings.value("trace_port", Server::default_port).toInt();
	m_profiler_addr = settings.value("profiler_addr", "127.0.0.1").toString();
	m_profiler_port = settings.value("profiler_port", 13147).toInt();
}

MainWindow::MainWindow (QWidget * parent, bool quit_delay)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m_settings(new Ui::SettingsDialog)
	, m_help(new Ui::HelpDialog)
#if (defined WIN32) && (defined STATIC)
	, m_hotkey(VK_SCROLL)
#endif
	, m_hidden(false)
	, m_was_maximized(false)
	, m_timer(new QTimer(this))
	, m_server(0)
	, m_minimize_action(0)
	, m_maximize_action(0)
	, m_restore_action(0)
	, m_quit_action(0)
	, m_tray_menu(0)
	, m_tray_icon(0)
	, m_settings_dialog(0)
	, ui_settings(0)
{
	//QDir::setSearchPaths("icons", QStringList(QDir::currentPath()));
	ui->setupUi(this);
	ui->tabTrace->setTabsClosable(true);

	QString const homedir = QDir::homePath();
	m_appdir = homedir + "/.flogging";

	// tray stuff
	createActions();
	createTrayIcon();
	QIcon icon(":images/Icon1.ico");
	setWindowIcon(icon);
	m_tray_icon->setVisible(true);
	m_tray_icon->show();

	setAcceptDrops(true);

	QSettings settings("MojoMir", "TraceServer");
	bool const on_top = settings.value("onTopCheckBox", true).toBool();
	if (on_top)
	{
		onOnTop(on_top);
	}
	ui->onTopCheckBox->setChecked(on_top);

	loadNetworkSettings();
	m_server = new Server(m_trace_addr, m_trace_port, this, quit_delay);
	showServerStatus();
	connect(ui->qSearchLineEdit, SIGNAL(editingFinished()), this, SLOT(onQSearchEditingFinished()));
	connect(ui->qFilterLineEdit, SIGNAL(editingFinished()), this, SLOT(onQFilterLineEditFinished()));

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
			ui->qSearchComboBox->addItem(qname);
		}
	}
	ui->qSearchComboBox->addItem(".*");
	ui->qSearchComboBox->setCurrentIndex(ui->qSearchComboBox->findText(msg_tag));


	m_timer->setInterval(5000);
	connect(m_timer, SIGNAL(timeout()) , this, SLOT(timerHit()));
	m_timer->start();
	setupMenuBar();

	getWidgetFile()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetFile(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtFileTree(QModelIndex)));
	connect(getWidgetFile(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtFileTree(QModelIndex)));

	getWidgetCtx()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetCtx(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtCtxTree(QModelIndex)));
	connect(getWidgetCtx(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtCtxTree(QModelIndex)));

	getWidgetTID()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetTID(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtTIDList(QModelIndex)));
	connect(getWidgetTID(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtTIDList(QModelIndex)));

	getWidgetLvl()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetLvl(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtLvlList(QModelIndex)));
	connect(getWidgetLvl(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtLvlList(QModelIndex)));

	connect(ui->levelSpinBox, SIGNAL(valueChanged(int)), m_server, SLOT(onLevelValueChanged(int)));
	connect(ui->filterFileCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onFilterFile(int)));
	connect(ui->buffCheckBox, SIGNAL(stateChanged(int)), m_server, SLOT(onBufferingStateChanged(int)));
	//connect(ui->onTopCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onOnTop(int)));
	connect(ui->reuseTabCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onReuseTabChanged(int)));
	//connect(ui->clrFiltersCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onClrFiltersStateChanged(int)));
	connect(ui->presetComboBox, SIGNAL(activated(int)), this, SLOT(onPresetActivate(int)));
	connect(ui->presetAddButton, SIGNAL(clicked()), this, SLOT(onAddCurrentFileFilter()));
	connect(ui->presetRmButton, SIGNAL(clicked()), this, SLOT(onRmCurrentFileFilter()));
	connect(ui->presetSaveButton, SIGNAL(clicked()), this, SLOT(onSaveCurrentFileFilter()));
	connect(ui->presetResetButton, SIGNAL(clicked()), m_server, SLOT(onClearCurrentFileFilter()));

	getWidgetRegex()->setEditTriggers(QAbstractItemView::NoEditTriggers);
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

	ui->autoScrollCheckBox->setToolTip(tr("auto scrolls to bottom if checked"));
	ui->reuseTabCheckBox->setToolTip(tr("reuses compatible tab instead of creating new one"));
	ui->clrFiltersCheckBox->setToolTip(tr("force clearing of filters when reuseTab is checked"));
	ui->scopesCheckBox->setToolTip(tr("hides scopes if checked"));
	ui->onTopCheckBox->setToolTip(tr("keeps window on top if checked. have to restart program, sorry"));
	ui->filterFileCheckBox->setToolTip(tr("enables filtering via fileFilter tab"));
	ui->buffCheckBox->setToolTip(tr("turns on/off buffering of messages on client side."));
	ui->presetComboBox->setToolTip(tr("selects and applies saved preset file filter"));
	ui->presetAddButton->setToolTip(tr("saves current fileFilter state as new preset"));
	ui->presetRmButton->setToolTip(tr("removes currently selected preset"));
	ui->presetSaveButton->setToolTip(tr("saves current fileFilter state as currently selected preset"));
	ui->presetResetButton->setToolTip(tr("clear current fileFilter"));
	ui->filterModeComboBox->setToolTip(tr("selects filtering mode: inclusive (check what you want, unchecked are not displayed) or exclusive (checked items are filtered out)."));
	ui->levelSpinBox->setToolTip(tr("adjusts debug level of client side"));
	ui->qSearchLineEdit->setToolTip(tr("search text in logged text"));
	ui->qFilterLineEdit->setToolTip(tr("quick inclusive filter: adds string to regex filter as regex .*string.*"));
	ui->qSearchComboBox->setToolTip(tr("specifies column to search"));


	m_status_label = new QLabel(m_server->getStatus());
	QLabel * version_label = new QLabel(tr("Ver: %1").arg(g_Version));
	statusBar()->addPermanentWidget(version_label);
	statusBar()->addWidget(m_status_label);

	connect(ui->filterModeComboBox, SIGNAL(activated(int)), this, SLOT(onFilterModeActivate(int)));
	connect(ui->tabTrace, SIGNAL(tabCloseRequested(int)), m_server, SLOT(onCloseTabWithIndex(int)));
	QTimer::singleShot(0, this, SLOT(loadState()));	// trigger lazy load of settings
	setWindowTitle(".*server");
}

MainWindow::~MainWindow()
{
#ifdef WIN32
	UnregisterHotKey(winId(), 0);
#endif
	delete m_help;
	delete m_settings;
	delete ui;
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
QTreeView * MainWindow::getWidgetFile () { return ui->treeViewFile; }
QTreeView const * MainWindow::getWidgetFile () const { return ui->treeViewFile; }
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
QListView * MainWindow::getWidgetLvl () { return ui->listViewLvl; }
QListView const * MainWindow::getWidgetLvl () const { return ui->listViewLvl; }

bool MainWindow::scopesEnabled () const { return ui->scopesCheckBox->isChecked(); }
bool MainWindow::onTopEnabled () const { return ui->onTopCheckBox->isChecked(); }
bool MainWindow::filterEnabled () const { return ui->filterFileCheckBox->isChecked(); }
bool MainWindow::reuseTabEnabled () const { return ui->reuseTabCheckBox->isChecked(); }
bool MainWindow::autoScrollEnabled () const { return ui->autoScrollCheckBox->isChecked(); }
bool MainWindow::buffEnabled () const { return ui->buffCheckBox->isChecked(); }
Qt::CheckState MainWindow::buffState () const { return ui->buffCheckBox->checkState(); }
bool MainWindow::clrFltEnabled () const { return ui->clrFiltersCheckBox->isChecked(); }
E_FilterMode MainWindow::fltMode () const
{
	return ui->filterModeComboBox->currentText() == QString("Inclusive") ? e_Include : e_Exclude;
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
	m_tray_icon->hide();
	storeState();
	qApp->quit();
}

void MainWindow::onReuseTabChanged (int state)
{
	ui->clrFiltersCheckBox->setEnabled(state);
}

void MainWindow::onFilterFile (int state)
{
	ui->filterModeComboBox->setEnabled(state);
	ui->presetComboBox->setEnabled(state);
	m_server->onFilterFile(state);
}

void MainWindow::onQSearchEditingFinished ()
{
	if (!getTabTrace()->currentWidget()) return;

	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QString text = ui->qSearchLineEdit->text();
	QString qcolumn = ui->qSearchComboBox->currentText();
	bool const search_all = (qcolumn == ".*");
	//qDebug("onQSearchEditingFinished: col=%s text=%s", qcolumn.toStdString().c_str(), text.toStdString().c_str());
	
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
		QList<QStandardItem *> row_items = addTriRow(text, true);
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

void MainWindow::onEditFind ()
{
	int const tab_idx = getTabTrace()->currentIndex();
	if (tab_idx < 0)
		return;

	bool ok;
	QString search = QInputDialog::getText(this, tr("Find"), tr("Text:"), QLineEdit::Normal, m_last_search, &ok);
	if (ok)
	{
		m_last_search = search;
		ui->qSearchLineEdit->setText(search);
		onQSearchEditingFinished();
	}
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
	getWidgetFile()->expandAll();
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
		Qt::WindowFlags const newflags = Qt::Window | (old & ~(Qt::WindowStaysOnTopHint | Qt::Tool));
		setWindowFlags(newflags);
	}
	else
	{
		setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);
	}
}

void MainWindow::onHotkeyShowOrHide ()
{
	bool const not_on_top = !isActiveWindow();
	qDebug("onHotkeyShowOrHide() isActive=%u", not_on_top);

	if (!m_hidden && not_on_top)
	{
		raise();
		activateWindow();
		return;
	}
	else
	{
		m_hidden = !m_hidden;
		if (m_hidden)
		{
			m_was_maximized = isMaximized();
			hide();
		}
		else
		{
			if (m_was_maximized)
				showMaximized();
			else
				showNormal();
			raise();
			activateWindow();
		}
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
		SessionExport se;
		conn->sessionState().sessionDump(se);
		session_string = QString("File filter:\n %1\nregex_filter:\n %2\nregex_inclusive:\n %3\ncolor_regexps:\n %4\ncolor_colors:\n %5\n")
				.arg(se.m_file_filters.c_str())
				.arg(se.m_regex_filters.c_str())
				.arg(se.m_regex_fmode.c_str())
				.arg(se.m_colortext_regexs.c_str())
				.arg(se.m_colortext_colors.c_str());
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

void MainWindow::onPresetActivate (int idx)
{
	if (idx == -1) return;
	if (!getTabTrace()->currentWidget()) return;

	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	loadSession(conn->m_session_state, m_preset_names.at(idx));

	//conn->onClearCurrentFileFilter();
	//conn->onClearCurrentColorizedRegexFilter();
	/*for (size_t i = 0, ie = m_filter_presets.at(idx).m_colortext_regexs.size(); i < ie; ++i)
	{
		std::string cregex_item(m_filter_presets.at(idx).m_colortext_regexs.at(i).toStdString());
		std::string cregex_col(m_filter_presets.at(idx).m_colortext_colors.at(i).toStdString());
		bool const enabled = m_filter_presets.at(idx).m_colortext_enabled.at(i).toInt();
		conn->loadToColorRegexps(cregex_item, cregex_col, enabled);

		QStandardItem * const root = static_cast<QStandardItemModel *>(getWidgetColorRegex()->model())->invisibleRootItem();
		QStandardItem * const child = findChildByText(root, m_filter_presets.at(idx).m_colortext_regexs.at(i));
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addRow(m_filter_presets.at(idx).m_colortext_regexs.at(i), enabled);
			root->appendRow(row_items);
		}
	}*/

	conn->recompileColorRegexps();
	/*for (size_t i = 0, ie = m_filter_presets.at(idx).m_regex_filters.size(); i < ie; ++i)
	{
		std::string regex_item(m_filter_presets.at(idx).m_regex_filters.at(i).toStdString());
		std::string regex_fmode(m_filter_presets.at(idx).m_regex_fmode.at(i).toStdString());
		bool const enabled = m_filter_presets.at(idx).m_regex_enabled.at(i).toInt();
		bool const inclusive = m_filter_presets.at(idx).m_regex_fmode.at(i).toInt();
		conn->loadToRegexps(regex_item, inclusive, enabled);

		QStandardItem * const root = static_cast<QStandardItemModel *>(getWidgetRegex()->model())->invisibleRootItem();
		QStandardItem * const child = findChildByText(root, m_filter_presets.at(idx).m_regex_filters.at(i));
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addTriRow(m_filter_presets.at(idx).m_regex_filters.at(i), inclusive);
			row_items.at(0)->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
			root->appendRow(row_items);
		}
	}*/
	conn->recompileRegexps();

	//getWidgetFile()->expandAll();
	conn->onInvalidateFilter();
}

void MainWindow::onFilterModeActivate (int idx)
{
	if (idx == -1) return;
	if (!getTabTrace()->currentWidget()) return;

	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;
	QString const qItem = ui->filterModeComboBox->currentText();

	qDebug("item=%s", qItem.toStdString().c_str());
	E_FilterMode const mode = qItem == "Inclusive" ? e_Include : e_Exclude;
	//@TODO: do following for each connection?
	conn->flipFilterMode(mode);
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
		QList<QStandardItem *> row_items = addTriRow(qItem, true);
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
	QStandardItem * root = static_cast<QStandardItemModel *>(getWidgetColorRegex()->model())->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(qItem, false);
		root->appendRow(row_items);

		conn->appendToColorRegexFilters(qItem.toStdString());
		conn->recompileColorRegexps();
	}
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
		txt = "default";
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
		{
			idx = addPresetName(preset_name);
		}
		qDebug("new preset_name[%i]=%s", idx, preset_name.toStdString().c_str());

		//saveSessionState(conn->sessionState(), "prvni.txt");
		/*SessionExport e;
		conn->sessionState().sessionExport(e);
		boost::char_separator<char> sep("\n");
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
		{
			m_filter_presets[idx].m_file_filters.clear();
			tokenizer_t tok(e.m_file_filters, sep);
			for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
				m_filter_presets[idx].m_file_filters << QString::fromStdString(*it);
		}

		{
			m_filter_presets[idx].m_colortext_regexs.clear();
			tokenizer_t tok(e.m_colortext_regexs, sep);
			for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
				m_filter_presets[idx].m_colortext_regexs << QString::fromStdString(*it);
		}
		{
			m_filter_presets[idx].m_colortext_colors.clear();
			tokenizer_t tok(e.m_colortext_colors, sep);
			for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
				m_filter_presets[idx].m_colortext_colors << QString::fromStdString(*it);
		}
		{
			m_filter_presets[idx].m_colortext_enabled.clear();
			tokenizer_t tok(e.m_colortext_enabled, sep);
			for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
				m_filter_presets[idx].m_colortext_enabled << QString::fromStdString(*it);
		}

		{
			m_filter_presets[idx].m_regex_filters.clear();
			tokenizer_t tok(e.m_regex_filters, sep);
			for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
				m_filter_presets[idx].m_regex_filters << QString::fromStdString(*it);
		}
		{
			m_filter_presets[idx].m_regex_fmode.clear();
			tokenizer_t tok(e.m_regex_fmode, sep);
			for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
				m_filter_presets[idx].m_regex_fmode << QString::fromStdString(*it);
		}
		{
			m_filter_presets[idx].m_regex_enabled.clear();
			tokenizer_t tok(e.m_regex_enabled, sep);
			for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
				m_filter_presets[idx].m_regex_enabled << QString::fromStdString(*it);
		}*/

		storePresets();

		ui->presetComboBox->clear();
		for (size_t i = 0, ie = m_preset_names.size(); i < ie; ++i)
			ui->presetComboBox->addItem(m_preset_names.at(i));
	}
}

void MainWindow::onAddCurrentFileFilter ()
{
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QString filled_text;
	filled_text.append(conn->sessionState().m_name);
	filled_text.append("/new_preset");

	QStringList items(m_preset_names);
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
	/*if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QString const text = ui->filterModeComboBox->currentText();
	int idx = findPresetName(text);
	if (idx == -1)
	{
		idx = addPresetName(text);
	}
	qDebug("removing preset_name[%i]=%s", idx, text.toStdString().c_str());
	
	ui->presetComboBox->clear();
	m_filter_presets[idx].clear();

	conn->onClearCurrentFileFilter();
	conn->onInvalidateFilter();

	//m_preset_names.at(idx) = QString("");
	storePresets();*/
}


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

	// Clear
	QMenu * clearMenu = menuBar()->addMenu(tr("&Clear"));
	clearMenu->addAction(tr("Clear current table view"), m_server, SLOT(onClearCurrentView()), QKeySequence(Qt::Key_C));
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_L), this, SLOT(onClearCurrentView()));
	clearMenu->addAction(tr("Clear current file filter"), m_server, SLOT(onClearCurrentFileFilter()));
	clearMenu->addAction(tr("Clear current context filter"), m_server, SLOT(onClearCurrentCtxFilter()));
	clearMenu->addAction(tr("Clear current thread id filter"), m_server, SLOT(onClearCurrentCtxFilter()));
	clearMenu->addAction(tr("Clear current colorized regexp filter"), m_server, SLOT(onClearCurrentColorizedRegexFilter()));
	clearMenu->addAction(tr("Clear current collapsed scope filter"), m_server, SLOT(onClearCurrentScopeFilter()));

	// Tools
	QMenu * tools = menuBar()->addMenu(tr("&Settings"));
	tools->addAction(tr("&Options"), this, SLOT(onSetup()), QKeySequence(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_O));
	//tools->addAction(tr("Save Current Filter As..."), this, SLOT(onSaveCurrentFileFilter()));
	tools->addSeparator();
	tools->addAction(tr("Save options now (this will NOT save presets)"), this, SLOT(storeState()), QKeySequence(Qt::AltModifier + Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_O));

	// Help
	QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(tr("Help"), this, SLOT(onShowHelp()), QKeySequence(Qt::Key_F1));
	helpMenu->addAction(tr("Dump filters"), this, SLOT(onDumpFilters()));
}

void write_list_of_strings (QSettings & settings, char const * groupname, char const * groupvaluename, QList<QString> const & lst)
{
	settings.beginWriteArray(groupname);
	for (int i = 0, ie = lst.size(); i < ie; ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue(groupvaluename, lst.at(i));
		qDebug("store to registry %i/%i: %s", i,ie, lst.at(i).toStdString().c_str());
	}
	settings.endArray();
}

void read_list_of_strings (QSettings & settings, char const * groupname, char const * groupvaluename, QList<QString> & lst)
{
	int const size = settings.beginReadArray(groupname);
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex(i);
		QString val = settings.value(groupvaluename).toString();
		qDebug("read from registry: %s", val.toStdString().c_str());
		lst.push_back(val);
	}
	settings.endArray();
}

void MainWindow::getPresetFileName (QString const & preset_name, QString & fname) const
{
	QString presetdir = m_appdir + "/" + preset_name;
	QDir d;
	d.mkpath(presetdir);
	fname = presetdir + "/session.state";
}

void MainWindow::saveSession (SessionState const & s, QString const & preset_name) const
{
	QString fname;
	getPresetFileName(preset_name, fname);
	qDebug("store file=%s", fname.toAscii());
	saveSessionState(s, fname.toAscii());
}

void MainWindow::storePresets ()
{
	qDebug("storePresets()");
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QSettings settings("MojoMir", "TraceServer");
	write_list_of_strings(settings, "known-presets", "preset", m_preset_names);

	for (size_t i = 0, ie = m_preset_names.size(); i < ie; ++i)
		if (!m_preset_names.at(i).isEmpty())
			saveSession(conn->sessionState(), m_preset_names.at(i));
}


bool MainWindow::loadSession (SessionState & s, QString const & preset_name)
{
	QString fname;
	getPresetFileName(preset_name, fname);
	qDebug("store file=%s", fname.toAscii());
	s.m_file_filters.clear();
	return loadSessionState(s, fname.toAscii());
}

void MainWindow::loadPresets ()
{
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	m_preset_names.clear();
	m_filter_presets.clear();
	QSettings settings("MojoMir", "TraceServer");
	read_list_of_strings(settings, "known-presets", "preset", m_preset_names);
	for (size_t i = 0, ie = m_preset_names.size(); i < ie; ++i)
	{
		ui->presetComboBox->addItem(m_preset_names.at(i));
	}

	for (size_t i = 0, ie = m_preset_names.size(); i < ie; ++i)
		if (!m_preset_names.at(i).isEmpty())
			loadSession(conn->m_session_state, m_preset_names.at(i));

	/* @TODO: read and transform into file
	for (size_t i = 0, ie = m_preset_names.size(); i < ie; ++i)
	{
		qDebug("reading preset: %s", m_preset_names.at(i).toStdString().c_str());
		m_filter_presets.push_back(Preset());
		settings.beginGroup(tr("preset_%1").arg(m_preset_names[i]));
		{
			read_list_of_strings(settings, "items", "item", m_filter_presets.back().m_file_filters);
			read_list_of_strings(settings, "cregexps", "item", m_filter_presets.back().m_colortext_regexs);
			read_list_of_strings(settings, "cregexps_colors", "item", m_filter_presets.back().m_colortext_colors);
			read_list_of_strings(settings, "cregexps_enabled", "item", m_filter_presets.back().m_colortext_enabled);
			read_list_of_strings(settings, "regexps", "item", m_filter_presets.back().m_regex_filters);
			read_list_of_strings(settings, "regexps_fmode", "item", m_filter_presets.back().m_regex_fmode);
			read_list_of_strings(settings, "regexps_enabled", "item", m_filter_presets.back().m_regex_enabled);
		}
		settings.endGroup();
	}*/
}

void MainWindow::storeState ()
{
	QSettings settings("MojoMir", "TraceServer");

	settings.setValue("trace_addr", m_trace_addr);
	settings.setValue("trace_port", m_trace_port);
	settings.setValue("profiler_addr", m_profiler_addr);
	settings.setValue("profiler_port", m_profiler_port);

	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
	settings.setValue("splitter", ui->splitter->saveState());
	settings.setValue("autoScrollCheckBox", ui->autoScrollCheckBox->isChecked());
	settings.setValue("reuseTabCheckBox", ui->reuseTabCheckBox->isChecked());
	settings.setValue("scopesCheckBox", ui->scopesCheckBox->isChecked());
	settings.setValue("onTopCheckBox", ui->onTopCheckBox->isChecked());
	settings.setValue("filterFileCheckBox", ui->filterFileCheckBox->isChecked());
	settings.setValue("buffCheckBox", ui->buffCheckBox->isChecked());
	settings.setValue("clrFiltersCheckBox", ui->clrFiltersCheckBox->isChecked());
	settings.setValue("filterModeComboBox", ui->filterModeComboBox->currentIndex());
	settings.setValue("levelSpinBox", ui->levelSpinBox->value());

	write_list_of_strings(settings, "known-applications", "application", m_app_names);
	for (size_t i = 0, ie = m_app_names.size(); i < ie; ++i)
	{
		settings.beginGroup(tr("column_order_%1").arg(m_app_names[i]));
		{
			write_list_of_strings(settings, "orders", "column", m_columns_setup.at(i));
		}
		settings.endGroup();

		settings.beginGroup(tr("column_sizes_%1").arg(m_app_names[i]));
		{
			QList<columns_sizes_t>::const_iterator oi = m_columns_sizes.constBegin();
			while (oi != m_columns_sizes.constEnd())
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

		settings.beginGroup(tr("column_align_%1").arg(m_app_names[i]));
		{
			write_list_of_strings(settings, "aligns", "column", m_columns_align.at(i));
		}
		settings.endGroup();

		settings.beginGroup(tr("column_elide_%1").arg(m_app_names[i]));
		{
			write_list_of_strings(settings, "elides", "column", m_columns_elide.at(i));
		}
		settings.endGroup();
	}

#ifdef WIN32
	settings.setValue("hotkeyCode", m_hotkey);
#endif
}

void MainWindow::loadState ()
{
	m_app_names.clear();
	m_columns_setup.clear();
	m_columns_sizes.clear();

	QSettings settings("MojoMir", "TraceServer");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	if (settings.contains("splitter"))
		ui->splitter->restoreState(settings.value("splitter").toByteArray());

	ui->autoScrollCheckBox->setChecked(settings.value("autoScrollCheckBox", true).toBool());
	ui->reuseTabCheckBox->setChecked(settings.value("reuseTabCheckBox", true).toBool());
	ui->scopesCheckBox->setChecked(settings.value("scopesCheckBox", false).toBool());
	ui->filterFileCheckBox->setChecked(settings.value("filterFileCheckBox", true).toBool());
	ui->buffCheckBox->setChecked(settings.value("buffCheckBox", true).toBool());
	ui->clrFiltersCheckBox->setChecked(settings.value("clrFiltersCheckBox", false).toBool());
	ui->filterModeComboBox->setCurrentIndex(settings.value("filterModeComboBox").toInt());
	ui->levelSpinBox->setValue(settings.value("levelSpinBox").toInt());

	read_list_of_strings(settings, "known-applications", "application", m_app_names);
	for (size_t i = 0, ie = m_app_names.size(); i < ie; ++i)
	{
		m_columns_setup.push_back(columns_setup_t());
		settings.beginGroup(tr("column_order_%1").arg(m_app_names[i]));
		{
			read_list_of_strings(settings, "orders", "column", m_columns_setup.back());
		}
		settings.endGroup();
		
		m_columns_sizes.push_back(columns_sizes_t());
		settings.beginGroup(tr("column_sizes_%1").arg(m_app_names[i]));
		{
			int const size = settings.beginReadArray("sizes");
			for (int i = 0; i < size; ++i) {
				settings.setArrayIndex(i);
				m_columns_sizes.back().push_back(settings.value("column").toInt());
			}
			settings.endArray();
		}
		settings.endGroup();

		m_columns_align.push_back(columns_align_t());
		settings.beginGroup(tr("column_align_%1").arg(m_app_names[i]));
		{
			read_list_of_strings(settings, "aligns", "column", m_columns_align.back());
		}
		settings.endGroup();

		if (m_columns_align.back().size() < m_columns_sizes.back().size())
			for (int i = 0, ie = m_columns_sizes.back().size(); i < ie; ++i)
				m_columns_align.back().push_back(QString("L"));

		m_columns_elide.push_back(columns_elide_t());
		settings.beginGroup(tr("column_elide_%1").arg(m_app_names[i]));
		{
			read_list_of_strings(settings, "elides", "column", m_columns_elide.back());
		}
		settings.endGroup();

		if (m_columns_elide.back().size() < m_columns_sizes.back().size())
			for (int i = 0, ie = m_columns_sizes.back().size(); i < ie; ++i)
				m_columns_elide.back().push_back(QString("R"));
	}

	if (m_thread_colors.empty())
	{
		for (size_t i = Qt::white; i < Qt::transparent; ++i)
			m_thread_colors.push_back(QColor(static_cast<Qt::GlobalColor>(i)));
	}

#ifdef WIN32
	unsigned const hotkeyCode = settings.value("hotkeyCode").toInt();
	m_hotkey = hotkeyCode ? hotkeyCode : VK_SCROLL;
	DWORD const hotkey = m_hotkey;
	int mod = 0;
	UnregisterHotKey(winId(), 0);
	RegisterHotKey(winId(), 0, mod, LOBYTE(hotkey));
#endif

	loadPresets();
	getWidgetFile()->setEnabled(filterEnabled());

	m_settings_dialog = new QDialog(this);
	m_settings_dialog->setWindowFlags(Qt::Sheet);
	ui_settings = new Ui::SettingsDialog();
	ui_settings->setupUi(m_settings_dialog);

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

	m_hidden = true;
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

