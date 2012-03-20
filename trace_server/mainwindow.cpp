#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settings.h"
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
#include <QUrl>
#include <QtPlugin>
#include "settings.h"
#include "../tlv_parser/tlv_parser.h"

#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

#if (defined WIN32) && (defined STATIC)
    Q_IMPORT_PLUGIN(qico);
    Q_IMPORT_PLUGIN(qsvg);
#endif

MainWindow::MainWindow (QWidget * parent, bool quit_delay)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m_settings(new Ui::SettingsDialog)
	, m_hidden(false)
	, m_timer(new QTimer(this))
	, m_server(0)
	, m_minimize_action(0)
	, m_maximize_action(0)
	, m_restore_action(0)
	, m_quit_action(0)
	, m_tray_menu(0)
	, m_tray_icon(0)

{
    //QDir::setSearchPaths("icons", QStringList(QDir::currentPath()));
	ui->setupUi(this);

	// tray stuff
	createActions();
	createTrayIcon();
    QIcon icon(":images/Icon1.ico");
	setWindowIcon(icon);
	m_tray_icon->setVisible(true);
	m_tray_icon->show();

	setAcceptDrops(true);
	qApp->installEventFilter(this);

	m_server = new Server(this, quit_delay);
	showServerStatus();
	connect(ui->qSearchLineEdit, SIGNAL(editingFinished()), this, SLOT(onQSearchEditingFinished()));

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

	getTreeViewFile()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getTreeViewFile(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtFileTree(QModelIndex)));
	connect(getTreeViewFile(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtFileTree(QModelIndex)));

	connect(getListViewTID(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtTIDList(QModelIndex)));
	connect(getListViewTID(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtTIDList(QModelIndex)));
	connect(ui->levelSpinBox, SIGNAL(valueChanged(int)), m_server, SLOT(onLevelValueChanged(int)));
    connect(ui->filterFileCheckBox, SIGNAL(stateChanged(int)), m_server, SLOT(onFilterFile(int)));
    connect(ui->buffCheckBox, SIGNAL(stateChanged(int)), m_server, SLOT(onBufferingStateChanged(int)));
	connect(ui->presetComboBox, SIGNAL(activated(int)), this, SLOT(onPresetActivate(int)));

	QTimer::singleShot(0, this, SLOT(loadState()));	// trigger lazy load of settings

	setWindowTitle("flog server");

	ui->autoScrollCheckBox->setToolTip(tr("auto scrolls to bottom if checked"));
	ui->reuseTabCheckBox->setToolTip(tr("reuses compatible tab instead of creating new one"));
	ui->scopesCheckBox->setToolTip(tr("hides scopes if checked"));
	ui->filterFileCheckBox->setToolTip(tr("enables filtering via fileFilter tab"));
	ui->buffCheckBox->setToolTip(tr("turns on/off buffering of messages on client side."));
	ui->presetComboBox->setToolTip(tr("selects and applies saved preset file filter"));
	ui->levelSpinBox->setToolTip(tr("adjusts debug level of client side"));
	ui->qSearchLineEdit->setToolTip(tr("search text in logged text"));
	ui->qSearchComboBox->setToolTip(tr("specifies column to search"));

#ifdef WIN32
	DWORD hotkey = VK_SCROLL;
	int mod = 0;
	UnregisterHotKey(winId(), 0);
	RegisterHotKey(winId(), 0, mod, LOBYTE(hotkey));
#endif
}

MainWindow::~MainWindow()
{
#ifdef WIN32
	UnregisterHotKey(winId(), 0);
#endif
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
QTreeView * MainWindow::getTreeViewFile () { return ui->treeViewFile; }
QTreeView const * MainWindow::getTreeViewFile () const { return ui->treeViewFile; }
QComboBox * MainWindow::getFilterRegex () { return ui->comboBoxRegex; }
QComboBox const * MainWindow::getFilterRegex () const { return ui->comboBoxRegex; }
QListView * MainWindow::getListViewTID () { return ui->listViewTID; }
QListView const * MainWindow::getListViewTID () const { return ui->listViewTID; }

bool MainWindow::scopesEnabled () const { return ui->scopesCheckBox->isChecked(); }
bool MainWindow::filterEnabled () const { return ui->filterFileCheckBox->isChecked(); }
bool MainWindow::reuseTabEnabled () const { return ui->reuseTabCheckBox->isChecked(); }
bool MainWindow::autoScrollEnabled () const { return ui->autoScrollCheckBox->isChecked(); }
bool MainWindow::buffEnabled () const { return ui->buffCheckBox->isChecked(); }

void MainWindow::setLevel (int i)
{
	bool const old = ui->levelSpinBox->blockSignals(true);
    ui->levelSpinBox->setValue(i);
	ui->levelSpinBox->blockSignals(old);
}

void MainWindow::onQuit ()
{
	m_tray_icon->hide();
	storeState();
	qApp->quit();
}

void MainWindow::onQSearchEditingFinished ()
{
	if (!getTabTrace()->currentWidget()) return;

	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QString text = ui->qSearchLineEdit->text();
	QString qcolumn = ui->qSearchComboBox->currentText();
	bool const search_all = (qcolumn == ".*");
	qDebug("onQSearchEditingFinished: col=%s text=%s", qcolumn.toStdString().c_str(), text.toStdString().c_str());
	
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
{
}

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

void MainWindow::onHotkeyShowOrHide ()
{
	m_hidden = !m_hidden;
	if (m_hidden)
	{
		qDebug("MainWindow::hide()");
		hide();
	}
	else
	{
		qDebug("MainWindow::show()");
		showNormal();
	}
}

void MainWindow::onColumnSetup ()
{
	QDialog dialog(this);
	dialog.setWindowFlags(Qt::Sheet);

	Ui::SettingsDialog sett_dialog;
	sett_dialog.setupUi(&dialog);

	QString tags("Available tags: ");
	size_t const n = tlv::get_tag_count();
	for (size_t i = tlv::tag_time; i < n; ++i)
	{
		char const * name = tlv::get_tag_name(i);
		if (name)
			tags += QString::fromStdString(name) + "  ";
	}
	sett_dialog.tagsLabel->setText(tags);
	SettingsModelView model(m_app_names, m_columns_setup);
	sett_dialog.tagsTableView->setModel(&model);
	sett_dialog.tagsTableView->horizontalHeader()->resizeSection(0, 100);
	sett_dialog.tagsTableView->horizontalHeader()->resizeSection(1, 400);
	sett_dialog.tagsTableView->setItemDelegate(new SettingsEditDelegate());

	if (dialog.exec() == QDialog::Accepted)
	{
		for (int i = 0, ie = model.m_app_names.size(); i < ie; ++i)
		{
			boost::char_separator<char> sep(", ");
			typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
			std::string s(model.m_rows[i].toStdString());
			tokenizer_t tok(s, sep);
			int const idx = findAppName(m_app_names[i]);
			qDebug("app=%s", m_app_names.at(i).toStdString().c_str());
			m_columns_setup[idx].clear();
			m_columns_sizes[idx].clear();
			if (idx >= 0)
			{
				size_t j = 0;
				for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it, ++j)
				{
					qDebug("  token=%s", it->c_str());
					m_columns_setup[idx].append(QString::fromStdString(*it));
					m_columns_sizes[idx].append(127);
				}
			}
		}
	}
}

void MainWindow::onFileFilterSetup ()
{
}

void MainWindow::onPresetActivate (int idx)
{
	if (idx == -1) return;
	if (!getTabTrace()->currentWidget()) return;

	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	conn->clearFilters();

	for (size_t i = 0, ie = m_filter_presets.at(idx).size(); i < ie; ++i)
	{
		std::string filter_item(m_filter_presets.at(idx).at(i).toStdString());
		conn->appendToFileFilters(filter_item, true);
		conn->sessionState().appendFileFilter(filter_item);
		conn->onInvalidateFilter();
	}
}

void MainWindow::onSaveCurrentFileFilter ()
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
	QString text = QInputDialog::getItem(this, tr("Save current preset"), tr("Preset name:"), items, 0, true, &ok);
	if (ok && !text.isEmpty())
	{
		int idx = findPresetName(text);
		if (idx == -1)
		{
			idx = addPresetName(text);
		}
		qDebug("new preset_name[%i]=%s", idx, text.toStdString().c_str());

		std::string current_filter;
		conn->sessionState().m_file_filters.export_filter(current_filter);

		boost::char_separator<char> sep("\n");
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
		tokenizer_t tok(current_filter, sep);
		m_filter_presets[idx].clear();
		for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
		{
			qDebug("appending to preset: %s", it->c_str());
			m_filter_presets[idx] << QString::fromStdString(*it);
		}

		storePresets();

		ui->presetComboBox->clear();
		for (size_t i = 0, ie = m_preset_names.size(); i < ie; ++i)
			ui->presetComboBox->addItem(m_preset_names.at(i));
	}
}

void MainWindow::setupMenuBar ()
{
	// File
	QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(tr("File &Load..."), this, SLOT(onFileLoad()), QKeySequence(Qt::ControlModifier + Qt::Key_L));
	fileMenu->addAction(tr("File &Save..."), this, SLOT(onFileSave()), QKeySequence(Qt::ControlModifier + Qt::Key_S));
	fileMenu->addAction(tr("File &Export (CSV)"), this, SLOT(onFileExportToCSV()), QKeySequence(Qt::ControlModifier + Qt::Key_E));
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

	// Tools
	QMenu * tools = menuBar()->addMenu(tr("&Settings"));
	tools->addAction(tr("Column Setup"), this, SLOT(onColumnSetup()));
	tools->addAction(tr("File Filter Setup"), this, SLOT(onFileFilterSetup()));
	tools->addAction(tr("Save Current File Filter As..."), this, SLOT(onSaveCurrentFileFilter()));
	tools->addSeparator();
	tools->addAction(tr("Save setup now"), this, SLOT(storeState()));
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

void MainWindow::storePresets ()
{
	qDebug("storePresets()");
	QSettings settings("MojoMir", "TraceServer");
	write_list_of_strings(settings, "known-presets", "preset", m_preset_names);

	for (size_t i = 0, ie = m_preset_names.size(); i < ie; ++i)
	{
		qDebug("store group=%s", m_preset_names.at(i).toStdString().c_str());
		settings.beginGroup(tr("preset_%1").arg(m_preset_names.at(i)));
		{
			write_list_of_strings(settings, "items", "item", m_filter_presets.at(i));
		}
		settings.endGroup();
	}
}

void MainWindow::loadPresets ()
{
	m_preset_names.clear();
	m_filter_presets.clear();
	QSettings settings("MojoMir", "TraceServer");
	read_list_of_strings(settings, "known-presets", "preset", m_preset_names);
	for (size_t i = 0, ie = m_preset_names.size(); i < ie; ++i)
	{
		ui->presetComboBox->addItem(m_preset_names.at(i));
	}

	for (size_t i = 0, ie = m_preset_names.size(); i < ie; ++i)
	{
		qDebug("reading preset: %s", m_preset_names.at(i).toStdString().c_str());
		m_filter_presets.push_back(filter_preset_t());
		settings.beginGroup(tr("preset_%1").arg(m_preset_names[i]));
		{
			read_list_of_strings(settings, "items", "item", m_filter_presets.back());
		}
		settings.endGroup();
	}
}

void MainWindow::storeState ()
{
	QSettings settings("MojoMir", "TraceServer");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
	settings.setValue("splitter", ui->splitter->saveState());
	settings.setValue("autoScrollCheckBox", ui->autoScrollCheckBox->isChecked());
	settings.setValue("reuseTabCheckBox", ui->reuseTabCheckBox->isChecked());
	settings.setValue("scopesCheckBox", ui->scopesCheckBox->isChecked());
	settings.setValue("filterFileCheckBox", ui->filterFileCheckBox->isChecked());
	settings.setValue("buffCheckBox", ui->buffCheckBox->isChecked());

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
	}
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

	ui->autoScrollCheckBox->setChecked(settings.value("autoScrollCheckBox", false).toBool());
	ui->reuseTabCheckBox->setChecked(settings.value("reuseTabCheckBox", false).toBool());
	ui->scopesCheckBox->setChecked(settings.value("scopesCheckBox", false).toBool());
	ui->filterFileCheckBox->setChecked(settings.value("filterFileCheckBox", false).toBool());
	ui->buffCheckBox->setChecked(settings.value("buffCheckBox", false).toBool());

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
	}

	if (m_thread_colors.empty())
	{
		for (size_t i = Qt::white; i < Qt::transparent; ++i)
			m_thread_colors.push_back(QColor(static_cast<Qt::GlobalColor>(i)));
	}

	loadPresets();

	getTreeViewFile()->setEnabled(filterEnabled());
}

void MainWindow::iconActivated (QSystemTrayIcon::ActivationReason reason)
{
	switch (reason) {
		case QSystemTrayIcon::Trigger:
			break;
		case QSystemTrayIcon::DoubleClick:
			onHotkeyShowOrHide();
			break;
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

void MainWindow::changeEvent (QEvent * e)
{
	QMainWindow::changeEvent(e);
}

bool MainWindow::eventFilter (QObject *, QEvent * e)
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

/*he QSocketNotifier class provides support for monitoring activity on a file descriptor.  */
