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
#include "delegates.h"
#include "tableview.h"

Server::Server (QString ip, unsigned short port, QObject * parent, bool quit_delay)
	: QTcpServer(parent)
{
	QHostAddress addr(ip);
	if (!listen(addr, port)) {
		status = tr("Unable to start server! Reason: %1").arg(errorString());
		if (quit_delay)
		{
			QMessageBox::critical(0, tr("ee"), status, QMessageBox::Ok, QMessageBox::Ok);	
			QTimer::singleShot(0, qApp, SLOT(quit()));
		}
		else
		{
			printf("Another instance is already running!\n");
			exit(0);
		}
		return;
	}

	status = tr("Server running at IP: %1 port: %2").arg(serverAddress().toString()).arg(serverPort());
}

Connection * Server::findCurrentConnection ()
{
	Q_ASSERT(parent());
	QWidget * w = static_cast<MainWindow *>(parent())->getTabTrace()->currentWidget();
	connections_t::iterator it = m_connections.find(w);
	return (it != m_connections.end()) ? it->second : 0;
}

Connection * Server::findConnectionByName (QString const & app_name)
{
	Q_ASSERT(parent());

	for (connections_t::const_iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
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

void Server::onTabTraceFocus (int i)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	QWidget * w = main_window->getTabTrace()->widget(i);
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
	{
		if (it->second->sessionState().m_tab_widget == w)
		{
			it->second->onTabTraceFocus();
			return;
		}
	}
}

Connection * Server::createNewTableView ()
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	Connection * connection = new Connection(this);
	connection->setMainWindow(main_window);
	QWidget * tab = new QWidget();
	QHBoxLayout * horizontalLayout = new QHBoxLayout(tab);
	horizontalLayout->setSpacing(6);
	horizontalLayout->setContentsMargins(11, 11, 11, 11);
	horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
	TableView * tableView = new TableView(tab);
	tableView->setItemDelegate(new TableItemDelegate(connection->sessionState(), connection));
	
	// to ignore 'resizeColumnToContents' when accidentaly double-clicked on header handle
	disconnect(tableView->horizontalHeader(), SIGNAL(sectionHandleDoubleClicked(int)), tableView, SLOT(resizeColumnToContents(int)));

	tableView->setObjectName(QString::fromUtf8("tableView"));
	ModelView * model = new ModelView(tableView, connection);
	disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), tableView->verticalHeader(), SLOT(sectionsInserted(QModelIndex,int,int)));
    //tableView->verticalHeader()->setFont(QFont(""));		// @TODO: into config
	tableView->verticalHeader()->setDefaultSectionSize(14);	// @TODO: into config
	tableView->verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance
	tableView->setModel(model);
	horizontalLayout->addWidget(tableView);
	connection->setTableViewWidget(tableView);
	connection->sessionState().setupThreadColors(main_window->getThreadColors());
	int const n = main_window->getTabTrace()->addTab(tab, QString::fromUtf8("???"));
	qDebug("created new tab at %u for connection @ 0x%08x", n, connection);

	connection->sessionState().setTabWidget(tab);
	connection->sessionState().setFilterMode(main_window->fltMode());

	if (main_window->filterEnabled())
	{
		connection->setFilterFile(Qt::Checked);
	}
	m_connections.insert(std::make_pair(tab, connection));
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
	main_window->statusBar()->showMessage(tr("Incomming tcp connection!"));
	emit newConnection(connection);
	
	// this is supposed to use blocking reads in own thread
	/*Connection * connection = createNewTableView ();
	connection->setSocketDescriptor(socketDescriptor);
    connect(connection, SIGNAL(finished()), connection, SLOT(deleteLater()));
	connection->start();*/
}

void Server::onShowPlots ()
{
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
	{
		it->second->onShowPlots();
	}
}
void Server::onHidePlots ()
{
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
	{
		it->second->onHidePlots();
	}
}

void Server::onClickedAtPlotTree (QModelIndex idx)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	TreeModel * model = static_cast<TreeModel *>(main_window->getWidgetPlots()->model());
	//QStandardItem * item = model->itemFromIndex(idx);
	//Q_ASSERT(item);

	QList<QString> path;
	QList<bool> state;
	path.push_front(model->data(idx).toString());
	state.push_front(model->data(idx, Qt::CheckStateRole).toBool());
	QModelIndex parent = model->parent(idx);
	while (parent.isValid())
	{
		path.push_front(model->data(parent).toString());
		parent = model->parent(parent);
	}

	if (Connection * conn = findConnectionByName(path.at(0)))
	{
		if (path.size() > 0)
			for (dataplots_t::iterator it = conn->m_dataplots.begin(), ite = conn->m_dataplots.end(); it != ite; ++it)
			{
				DataPlot * dp = (*it);
				if (dp->m_config.m_tag == path.at(1))
				{
					break;
				}
			}
	}

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
			onCloseTab(idx, w);
		}
	}
}
void Server::onCloseTabWithIndex (int idx)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	if (idx != -1)
	{
		if (QWidget * w = main_window->getTabTrace()->widget(idx))
		{
			qDebug("Server::onCloseTabWithIndex(int) idx=%i widget=0x%08x", idx, w);
			onCloseTab(idx, w);
		}
	}
}
void Server::onCloseCurrentTab ()
{
	qDebug("%s", __FUNCTION__);
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	QWidget * w = main_window->getTabTrace()->currentWidget();
	onCloseTab(w);
}

void Server::destroyConnection (Connection * connection)
{
	QObject::disconnect(connection->m_tcpstream, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	QObject::disconnect(connection->m_tcpstream, SIGNAL(readyRead()), connection, SLOT(processReadyRead()));
	QObject::disconnect(connection->m_table_view_widget->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));

	delete connection;
}

void Server::onCloseTab (int idx, QWidget * w)
{
	qDebug("Server::onCloseTab(idx=%i, QWidget *=0x%08x) idx=%i", idx, w, idx);
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	connections_t::iterator it = m_connections.find(w);
	if (it != m_connections.end())
	{
		Connection * connection = it->second;
		m_connections.erase(it);
		destroyConnection(connection);
	}
	main_window->getTabTrace()->removeTab(idx);
	qDebug("Server::onCloseTab(idx=%i, QWidget *=0x%08x) curr idx=%i", idx, w, main_window->getTabTrace()->currentIndex());
	onTabTraceFocus(main_window->getTabTrace()->currentIndex());
}
void Server::onCloseMarkedTabs ()
{
	qDebug("%s", __FUNCTION__);

	std::vector<QWidget *> to_delete;
	to_delete.reserve(m_connections.size());

	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
	{
		if (it->second->m_marked_for_close)
			to_delete.push_back(it->first);
	}

	while (!to_delete.empty())
	{
		onCloseTab(to_delete.back());
		to_delete.pop_back();
	}
}

profiler::ProfilerWindow * Server::createNewProfilerView ()
{
	return 0;
}

void Server::incomingProfilerConnection (profiler::profiler_rvp_t * rvp)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());

	using namespace profiler;
	ProfilerWindow * w = new ProfilerWindow(this, rvp);
	qDebug("Incomming profiler rendez-vous point!");
	main_window->statusBar()->showMessage(tr("Incomming profiler rendez-vous point!"));
}

