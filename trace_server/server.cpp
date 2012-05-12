#include <QApplication>
#include <QClipboard>
#include <QtGui>
#include <QtNetwork/QtNetwork>
#include <stdlib.h>
#include "server.h"
#include "connection.h"
#include "mainwindow.h"
#include "modelview.h"
#include "utils.h"

void SaveTableViewSettings (QTableView * tb)
{
	QSettings settings("MojoMir", "TraceServer");
	QByteArray state = tb->horizontalHeader()->saveState();
	settings.setValue("column_width", state);
}

Server::Server (QObject * parent, bool quit_delay)
	: QTcpServer(parent)
{
	QHostAddress addr(QHostAddress::LocalHost);
	if (!listen(addr, default_port)) {
		status = tr("Unable to start server! Reason: %1").arg(errorString());
		if (quit_delay)
			QTimer::singleShot(5000, qApp, SLOT(quit()));
		else
		{
			printf("Another instance already running!\n");
			exit(0);
			//QTimer::singleShot(0, qApp, SLOT(quit()));
		}
		return;
	}

	status = tr("Server running at IP: %1 port: %2").arg(serverAddress().toString()).arg(serverPort());
}

Connection * Server::findCurrentConnection ()
{
	Q_ASSERT(parent());
	QWidget * w = static_cast<MainWindow *>(parent())->getTabTrace()->currentWidget();
	connections_t::iterator it = connections.find(w);
	return (it != connections.end()) ? it->second : 0;
}

Connection * Server::findConnectionByName (QString const & app_name)
{
	Q_ASSERT(parent());

	for (connections_t::const_iterator it = connections.begin(), ite = connections.end(); it != ite; ++it)
		if (it->second->sessionState().m_name == app_name)
			return it->second;
	return 0;
}

void Server::copyStorageTo (QString const & filename)
{
	if (Connection * conn = findCurrentConnection())
		conn->copyStorageTo(filename);
}

void Server::exportStorageToCSV (QString const & filename)
{
	if (Connection * conn = findCurrentConnection())
		conn->exportStorageToCSV(filename);
}

void Server::onSectionResized (int idx, int /*old_size*/, int new_size)
{
	if (Connection * conn = findCurrentConnection())
		if (conn->sessionState().getColumnSizes() && idx < conn->sessionState().getColumnSizes()->size())
			conn->sessionState().getColumnSizes()->operator[](idx) = new_size;
}

void Server::onLevelValueChanged (int val)
{
	qDebug("level changed: %u", val);
	if (Connection * conn = findCurrentConnection())
		conn->onLevelValueChanged(val);
}

void Server::onEditingFinished ()
{
}

void Server::onCopyToClipboard ()
{
	if (Connection * conn = findCurrentConnection())
	{
		QString selection = conn->onCopyToClipboard();
        qApp->clipboard()->setText(selection);
	}
}

void Server::onFilterFile (int state)
{
	if (Connection * conn = findCurrentConnection())
		conn->setFilterFile(state);
}

void Server::onBufferingStateChanged (int state)
{
	if (Connection * conn = findCurrentConnection())
		conn->onBufferingStateChanged(state);
}

void Server::onClearCurrentView ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onClearCurrentView();
}

void Server::onHidePrevFromRow ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onHidePrevFromRow();
}

void Server::onUnhidePrevFromRow ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onUnhidePrevFromRow();
}

void Server::onExcludeFileLine ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onExcludeFileLine();
}

void Server::onToggleRefFromRow ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onToggleRefFromRow();
}

void Server::onClickedAtFileTree_Impl (QModelIndex idx, bool recursive)
{
	MainWindow * const main_window = static_cast<MainWindow *>(parent());
	std::vector<QString> s;
	s.reserve(16);
	QStandardItemModel const * const model = static_cast<QStandardItemModel *>(main_window->getTreeViewFile()->model());
	QStandardItem * const item = model->itemFromIndex(idx);

	E_FilterMode const fmode = main_window->fltMode();
	bool const orig_checked = (item->checkState() == Qt::Checked);

	setCheckState(item, !orig_checked);
	setCheckStateRecursive(item, !orig_checked);

	QStandardItem const * line_item = 0;
	if (!item->hasChildren())
		line_item = item;
	else
		s.push_back(model->data(idx, Qt::DisplayRole).toString());

	QStandardItem * parent = item->parent();
	std::string file;
	QModelIndex parent_idx = model->indexFromItem(parent);
	while (parent_idx.isValid())
	{
		QString const & val = model->data(parent_idx, Qt::DisplayRole).toString();
		s.push_back(val);
		parent = parent->parent();
		parent_idx = model->indexFromItem(parent);
	}

	for (std::vector<QString>::const_reverse_iterator it=s.rbegin(), ite=s.rend(); it != ite; ++it)
		file += std::string("/") + (*it).toStdString();
	//qDebug("file=%s", file.c_str());

	bool checked = (item->checkState() == Qt::Checked);

	fileline_t filter_item(file, std::string());
	if (line_item)
	{
		QString const & val = model->data(idx, Qt::DisplayRole).toString();
		filter_item.second = val.toStdString();
	}

	if (Connection * const conn = findCurrentConnection())
	{
		if (fmode == e_Include)
			checked = !checked;
		qDebug("file click! (checked=%u) %s: %s/%s", checked, (checked ? "append" : "remove"), filter_item.first.c_str(), filter_item.second.c_str());
		if (checked)
			conn->sessionState().appendFileFilter(filter_item);
		else
			conn->sessionState().removeFileFilter(filter_item);

		QStandardItem * n = item->parent();
		while (n)
		{
			n->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
			n = n->parent();
		}
		conn->onInvalidateFilter();
	}
}
void Server::onClickedAtFileTree (QModelIndex idx)
{
	onClickedAtFileTree_Impl(idx, false);
}

void Server::onDoubleClickedAtFileTree (QModelIndex idx)
{
	//MainWindow * main_window = static_cast<MainWindow *>(parent());
	//QStandardItemModel * model = static_cast<QStandardItemModel *>(main_window->getTreeViewFile()->model());
	//QStandardItem * item = model->itemFromIndex(idx);
	//onClickedAtFileTree_Impl(idx, true);
}

void Server::onClickedAtCtxTree (QModelIndex idx)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	QStandardItemModel * model = static_cast<QStandardItemModel *>(main_window->getTreeViewCtx()->model());
	QStandardItem * item = model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & val = model->data(idx, Qt::DisplayRole).toString();
	std::string const ctx_item(val.toStdString());
	context_t const ctx = val.toULongLong();

	bool const checked = (item->checkState() == Qt::Checked);

	if (Connection * conn = findCurrentConnection())
	{
		if (checked)
			conn->sessionState().appendCtxFilter(ctx);
		else
			conn->sessionState().removeCtxFilter(ctx);
		conn->onInvalidateFilter();
	}
}

void Server::onDoubleClickedAtCtxTree (QModelIndex idx)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	QStandardItemModel * model = static_cast<QStandardItemModel *>(main_window->getTreeViewCtx()->model());
	QStandardItem * item = model->itemFromIndex(idx);

	bool const checked = (item->checkState() == Qt::Checked);
	item->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
	onClickedAtCtxTree(idx);
}

void Server::onClickedAtTIDList (QModelIndex idx)
{
	if (!idx.isValid())
		return;
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	QStandardItemModel * model = static_cast<QStandardItemModel *>(main_window->getListViewTID()->model());
	QStandardItem * item = model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & val = model->data(idx, Qt::DisplayRole).toString();
	std::string filter_item(val.toStdString());

	bool const checked = (item->checkState() == Qt::Checked);

	if (Connection * conn = findCurrentConnection())
	{
		if (checked)
			conn->sessionState().appendTIDFilter(filter_item);
		else
			conn->sessionState().removeTIDFilter(filter_item);
		conn->onInvalidateFilter();
	}
}

void Server::onDoubleClickedAtTIDList (QModelIndex idx)
{ }

void Server::onClickedAtRegexList (QModelIndex idx)
{
	if (!idx.isValid())
		return;
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	QStandardItemModel * model = static_cast<QStandardItemModel *>(main_window->getListViewRegex()->model());
	QStandardItem * item = model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & val = model->data(idx, Qt::DisplayRole).toString();
	std::string filter_item(val.toStdString());

	bool const checked = (item->checkState() == Qt::Checked);

	qDebug("regex click! (checked=%u) %s ", checked, filter_item.c_str());

	if (Connection * conn = findCurrentConnection())
	{
		// @TODO: if state really changed
		main_window->recompileRegexps();
		conn->onInvalidateFilter();
	}
}

void Server::onDoubleClickedAtRegexList (QModelIndex idx)
{ }

void Server::onClickedAtColorRegexList (QModelIndex idx)
{
	if (!idx.isValid()) return;
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	QStandardItemModel * model = static_cast<QStandardItemModel *>(main_window->getListViewColorRegex()->model());
	QStandardItem * item = model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & val = model->data(idx, Qt::DisplayRole).toString();
	std::string filter_item(val.toStdString());

	bool const checked = (item->checkState() == Qt::Checked);
	qDebug("color regex click! (checked=%u) %s ", checked, filter_item.c_str());

	if (Connection * conn = findCurrentConnection())
	{
		// @TODO: if state really changed
		main_window->recompileColorRegexps();
		conn->onInvalidateFilter();
		conn->m_session_state.setColorRegexChecked(filter_item, checked);
	}
}

void Server::onDoubleClickedAtColorRegexList (QModelIndex idx)
{
	if (!idx.isValid())
		return;
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	QStandardItemModel * model = static_cast<QStandardItemModel *>(main_window->getListViewColorRegex()->model());
	QStandardItem * item = model->itemFromIndex(idx);
	Q_ASSERT(item);
	QString const & val = model->data(idx, Qt::DisplayRole).toString();

    QColor color = QColorDialog::getColor(Qt::black);
	if (!color.isValid())
		return;

	if (Connection * conn = findCurrentConnection())
	{
		conn->m_session_state.setRegexColor(val.toStdString(), color);
		main_window->recompileRegexps();
	}
}

Connection * Server::createNewTableView ()
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	Connection * connection = new Connection(this);
	connection->setMainWindow(main_window);
	connection->setupModelFile();
	connection->setupModelCtx();
	connection->setupModelTID();
	connection->setupModelColorRegex();
	QWidget * tab = new QWidget();
	QHBoxLayout * horizontalLayout = new QHBoxLayout(tab);
	horizontalLayout->setSpacing(6);
	horizontalLayout->setContentsMargins(11, 11, 11, 11);
	horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
	QTableView * tableView = new QTableView(tab);
	tableView->setObjectName(QString::fromUtf8("tableView"));
	ModelView * model = new ModelView(tableView, connection);
    //tableView->verticalHeader()->setFont(QFont(""));
	tableView->verticalHeader()->setDefaultSectionSize(14);
	tableView->setModel(model);
	horizontalLayout->addWidget(tableView);
	connection->setTableViewWidget(tableView);
	connection->sessionState().setupThreadColors(main_window->getThreadColors());
	int const n = main_window->getTabTrace()->addTab(tab, QString::fromUtf8("???"));
	qDebug("created new tab at %u for connection @ 0x%08x", n, connection);

	connection->sessionState().setTabWidget(n);
	connection->sessionState().setTabWidget(tab);
	connection->sessionState().setFilterMode(main_window->fltMode());

	if (main_window->filterEnabled())
	{
		connection->setFilterFile(Qt::Checked);
	}
	main_window->getTabTrace()->setCurrentIndex(n);
	connections.insert(std::make_pair(tab, connection));
	QObject::connect(main_window->getTabTrace(), SIGNAL(currentChanged(int)), connection, SLOT(onTabTraceFocus(int)));
	QObject::connect(tableView->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));
	return connection;
}

void Server::incomingDataStream (QDataStream & stream)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	Connection * connection = createNewTableView ();
	main_window->statusBar()->showMessage(tr("Importing file!"));
	connection->processDataStream(stream);
}

void Server::incomingConnection (int socketDescriptor)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	Connection * connection = createNewTableView ();
	connection->setSocketDescriptor(socketDescriptor);
	QObject::connect(connection->m_tcpstream, SIGNAL(readyRead()), connection, SLOT(processReadyRead()));
	QObject::connect(connection->m_tcpstream, SIGNAL(disconnected()), connection, SLOT(onDisconnected()));
	main_window->statusBar()->showMessage(tr("Incomming connection!"));
	emit newConnection(connection);
	
	// this is supposed to use blocking reads in own thread
	/*Connection * connection = createNewTableView ();
	connection->setSocketDescriptor(socketDescriptor);
    connect(connection, SIGNAL(finished()), connection, SLOT(deleteLater()));
	connection->start();*/
}

void Server::onCloseTab (QWidget * w)
{
	if (w)
	{
		MainWindow * main_window = static_cast<MainWindow *>(parent());
		int const idx = main_window->getTabTrace()->indexOf(w);
		if (idx != -1)
		{
			qDebug("Server::onCloseTab(QWidget *) idx=%i", idx);
			main_window->getTabTrace()->removeTab(idx);
			connections_t::iterator it = connections.find(w);
			if (it != connections.end())
			{
				Connection * connection = it->second;

				QObject::disconnect(connection->m_table_view_widget->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));
				QObject::disconnect(main_window->getTabTrace(), SIGNAL(currentChanged(int)), connection, SLOT(onTabTraceFocus(int)));

				connection->onCloseTab();
				connections.erase(it);
				delete connection;
			}
		}
	}
}

void Server::onCloseCurrentTab ()
{
	qDebug("Server::onCloseCurrentTab");
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	QWidget * w = main_window->getTabTrace()->currentWidget();
	onCloseTab(w);
}

