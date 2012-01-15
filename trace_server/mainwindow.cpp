#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settings.h"
#include "modelview.h"
#include "server.h"
#include <QTime>
#include <QTableView>
#include <QShortcut>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QMetaType>
#include <QVariant>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QUrl>
#include "../tlv_parser/tlv_parser.h"

#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

Q_DECLARE_METATYPE(QList<int>)

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m_settings(new Ui::SettingsDialog)
	, m_hidden(false)
	, m_timer(new QTimer(this))
	, m_server(0)
{
	ui->setupUi(this);
	setAcceptDrops(true);
	//qApp->installEventFilter(this);

	m_server = new Server(this);
	showServerStatus();
	connect(ui->lineEdit, SIGNAL(editingFinished()), m_server, SLOT(onEditingFinished()));

	m_timer->setInterval(3000);
	connect(m_timer, SIGNAL(timeout()) , this, SLOT(timerHit()));
	m_timer->start();
	setupMenuBar();

	getTreeViewFile()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getTreeViewFile(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtFileTree(QModelIndex)));
	connect(ui->applyButton, SIGNAL(clicked()), m_server, SLOT(onApplyFilterClicked()));
	connect(ui->levelSpinBox, SIGNAL(valueChanged(int)), m_server, SLOT(onLevelValueChanged(int)));
    connect(ui->filterFileCheckBox, SIGNAL(stateChanged(int)), m_server, SLOT(onFilterFile(int)));

	QTimer::singleShot(0, this, SLOT(loadState()));	// trigger lazy load of settings

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
QTreeView * MainWindow::getTreeViewFunc () { return ui->treeViewFunc; }
QTreeView const * MainWindow::getTreeViewFunc () const { return ui->treeViewFunc; }

bool MainWindow::scopesEnabled () const { return ui->scopesCheckBox->isChecked(); }

void MainWindow::onEditFind ()
{
	int const tab_idx = getTabTrace()->currentIndex();
	if (tab_idx < 0)
		return;

	bool ok;
	QString search = QInputDialog::getText(this, tr("Find"), tr("Text:"), QLineEdit::Normal, m_last_search, &ok);
	if (ok && !search.isEmpty())
	{
		m_last_search = search;
		//if (!getTabTrace()->currentWidget()->findText(m_last_search))
		//	  slotUpdateStatusbar(tr("\"%1\" not found.").arg(m_last_search));
	}
}

void MainWindow::onEditFindNext ()
{
/*
	if (!currentTab() && !m_lastSearch.isEmpty())
		return;
	currentTab()->findText(m_lastSearch);*/
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
		show();
	}
}

namespace {
	struct SettingsModelView : QAbstractTableModel
	{
		//Q_OBJECT
	public:
		explicit SettingsModelView (QList<QString> const & app_names,
					QList<MainWindow::columns_setup_t> const & col_setup)
			: QAbstractTableModel(0)
			, m_app_names(app_names)
			, m_rows()
		{
			for (size_t i = 0, ie = app_names.size(); i < ie; ++i)
			{
				m_rows.push_back(QString());

				for (size_t j = 0, je = col_setup.at(i).size(); j < je; ++j)
					m_rows.back() += col_setup.at(i).at(j) + QString(", ");
			}
		}
		virtual ~SettingsModelView () { }
		int rowCount (const QModelIndex & parent = QModelIndex()) const { return m_app_names.size(); }
		int columnCount (const QModelIndex & parent = QModelIndex()) const { return 2; }
		QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const
		{
			if (role == Qt::DisplayRole)
			{
				QString str("");
				if (index.column() == 0)
					if (index.row() < m_app_names.size())
						return m_app_names[index.row()];
				if (index.column() == 1)
                    if (index.row() < (int)m_rows.size())
						return m_rows[index.row()];
				return str;
			}
			return QVariant();
		}
		QVariant headerData (int section, Qt::Orientation orientation, int role) const
		{
			if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
			{
				if (section == 0)
					return QString("Application name");
				if (section == 1)	
					return QString("tags separated by ,");
			}
			return QVariant();
		}

	signals:
		
	public slots:

	private:

		QList<QString> const & m_app_names;
		std::vector<QString> m_rows;
	};
}
void MainWindow::onSettings ()
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
	//sett_dialog.iconLabel->setText(QString());
   // sett_dialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32,

	//QString introMessage = tr("<qt>Connect to proxy \"%1\" using:</qt>");
	//introMessage = introMessage.arg(Qt::escape(proxy.hostName()));
	//proxyDialog.introLabel->setText(introMessage);
	//proxyDialog.introLabel->setWordWrap(true);

	if (dialog.exec() == QDialog::Accepted) {
		//auth->setUser(proxyDialog.userNameLineEdit->text());
		//auth->setPassword(proxyDialog.passwordLineEdit->text());
	}

}

void MainWindow::setupMenuBar ()
{
	// File
	QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(tr("File &Load..."), this, SLOT(onFileLoad()), QKeySequence(Qt::ControlModifier + Qt::Key_L));
	fileMenu->addAction(tr("File &Save..."), this, SLOT(onFileSave()), QKeySequence(Qt::ControlModifier + Qt::Key_S));
	fileMenu->addAction(tr("File &Export (CSV)"), this, SLOT(onFileExportToCSV()), QKeySequence(Qt::ControlModifier + Qt::Key_E));

	// Edit
	QMenu * editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(tr("Find"), this, SLOT(onEditFind()), QKeySequence::Find);
	editMenu->addAction(tr("Find Next"), this, SLOT(onEditFindNext()), QKeySequence::FindNext);
	editMenu->addAction(tr("Find Prev"), this, SLOT(onEditFindPrev()), QKeySequence::FindPrevious);
	new QShortcut(QKeySequence(Qt::Key_Slash), this, SLOT(onEditFind()));
	editMenu->addSeparator();
    //editMenu->addAction(tr("Kopy"), m_server, SLOT(onCopyToClipboard()), QKeySequence(Qt::ControlModifier + Qt::Key_K));
    editMenu->addAction(tr("Copy"), m_server, SLOT(onCopyToClipboard()), QKeySequence::Copy);

	QMenu * tools = menuBar()->addMenu(tr("&Tools"));
	tools->addAction(tr("Settings"), this, SLOT(onSettings()));
	new QShortcut(QKeySequence(Qt::Key_ScrollLock), this, SLOT(onHotkeyShowOrHide()));
}

void MainWindow::storeState ()
{
	QSettings settings("MojoMir", "TraceServer");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
	settings.setValue("splitter", ui->splitter->saveState());

	settings.beginWriteArray("known-applications");
	int const size = m_app_names.size();
	for (int i = 0; i < size; ++i) {
		settings.setArrayIndex(i);
		settings.setValue("application", m_app_names.at(i));
	}
	settings.endArray();

	for (size_t i = 0, ie = m_app_names.size(); i < ie; ++i)
	{
		settings.beginGroup(tr("column_order_%1").arg(m_app_names[i]));
		{
			QList<columns_setup_t>::const_iterator oi = m_columns_setup.constBegin();
			while (oi != m_columns_setup.constEnd())
			{
				settings.beginWriteArray("orders");
				int const size = (*oi).size();
				for (int i = 0; i < size; ++i) {
					settings.setArrayIndex(i);
					settings.setValue("column", (*oi).at(i));
				}
				settings.endArray();
				++oi;
			}
		}
		settings.endGroup();

		settings.beginGroup(tr("column_sizes_%1").arg(m_app_names[i]));
		{
			QList<columns_sizes_t>::const_iterator oi = m_columns_sizes.constBegin();
			while (oi != m_columns_sizes.constEnd())
			{
				settings.beginWriteArray("sizes");
				int const size = (*oi).size();
				for (int i = 0; i < size; ++i) {
					settings.setArrayIndex(i);
					settings.setValue("column", QString::number((*oi).at(i)));
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
	QSettings settings("MojoMir", "TraceServer");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	if (settings.contains("splitter"))
		ui->splitter->restoreState(settings.value("splitter").toByteArray());

	m_app_names.clear();
	int const size = settings.beginReadArray("known-applications");
	for (int i = 0; i < size; ++i) {
		settings.setArrayIndex(i);
		m_app_names.append(settings.value("application").toString());
	}
	settings.endArray();

	m_columns_setup.clear();
	for (size_t i = 0, ie = m_app_names.size(); i < ie; ++i)
	{
		m_columns_setup.push_back(columns_setup_t());
		settings.beginGroup(tr("column_order_%1").arg(m_app_names[i]));
		{
			int const size = settings.beginReadArray("orders");
			for (int i = 0; i < size; ++i) {
				settings.setArrayIndex(i);
				QString val = settings.value("column").toString();
				m_columns_setup.back().push_back(val);
			}
			settings.endArray();
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
}

void MainWindow::closeEvent (QCloseEvent * event)
{
	storeState();
	QMainWindow::closeEvent(event);
}

void MainWindow::changeEvent (QEvent * e)
{
	switch (e->type())
	{
		case QEvent::LanguageChange:
			this->ui->retranslateUi(this);
			break;
		case QEvent::WindowStateChange:
			{
				if (this->windowState() & Qt::WindowMinimized)
				{
					//if (Preferences::instance().minimizeToTray())
					{
						QTimer::singleShot(250, this, SLOT(hide()));
					}
				}

				break;
			}
		default:
			break;
	}
	QMainWindow::changeEvent(e);
}

/*bool MainWindow::eventFilter (QObject * o, QEvent * e)
{
	//	qDebug() << e->type() << o;
	if (e->type() == QEvent::DragEnter)
	{
		qDebug("QEvent::DragEnter");
		dynamic_cast<QDragEnterEvent *>(e)->acceptProposedAction();
	}
	
	if (e->type() == QEvent::Drop)
	{
		qDebug("QEvent::Drop");
	}
	return false;
}*/

/*he QSocketNotifier class provides support for monitoring activity on a file descriptor.  */
/* The Qt Resource System ... for storing icons etc */
