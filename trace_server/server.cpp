#ifdef WIN32
#	define _WINSOCKAPI_ 
#endif
#include <QApplication>
#include <QClipboard>
#include <QtGui>
#include <QMessageBox>
#include <QStatusBar>
#include <QtNetwork/QtNetwork>
#include <stdlib.h>
#include "server.h"
#include "connection.h"
#include "mainwindow.h"
#include "logs/logtablemodel.h"
#include "utils.h"
#include "tableview.h"

Server::Server (QString ip, unsigned short port, QObject * parent, bool quit_delay)
	: QTcpServer(parent)
	, m_main_window(0)
{
		m_main_window = static_cast<MainWindow *>(parent);
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
	QWidget * w = m_main_window->getTabTrace()->currentWidget();
	connections_t::iterator it = m_connections.find(w);
	return (it != m_connections.end()) ? it->second : 0;
}

Connection * Server::findConnectionByName (QString const & app_name)
{
	Q_ASSERT(parent());

	for (connections_t::const_iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		if (it->second->sessionState().getAppName() == app_name)
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
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		it->second->onBufferingStateChanged(state);
}

void Server::onTabTraceFocus (int i)
{
	QWidget * w = m_main_window->getTabTrace()->widget(i);
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
	{
		if (it->second->m_tab_widget == w)
		{
			it->second->onTabTraceFocus();
			return;
		}
	}
}

Connection * Server::createNewTableView ()
{
	Connection * connection = new Connection(this);
	QWidget * tab = new QWidget();
	connection->m_tab_widget = tab;
	connection->setMainWindow(m_main_window);

	int const n = m_main_window->getTabTrace()->addTab(tab, QString::fromUtf8("???"));
	qDebug("created new tab at %u for connection @ 0x%08x", n, connection);

	//QString const tag("default"); // @FIXME
	//datalogs_t::iterator it = connection->findOrCreateLog(tag);
	//DataLog & d = **it;

	///////////////////////////////
	//prestehovano do DataLog::DataLog
	///////////////////////////////


	/*if (m_main_window->filterEnabled())
	{
		connection->setFilterFile(Qt::Checked);
	}*/
	m_connections.insert(std::make_pair(tab, connection));
	return connection;
}

void Server::importDataStream (QString const & fname)
{
	QFile file(fname);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning(QString(tr("Could not open file %1").arg(fname)).toLatin1());
		return;
	}

	Connection * connection = createNewTableView ();
	connection->setImportFile(fname);
	m_main_window->statusBar()->showMessage(tr("Importing file!"));

	QDataStream import_stream(&file);
	connection->processDataStream(import_stream);
	file.close();
}

void Server::createTailLogStream (QString const & fname, QString const & separator)
{
	Connection * connection = createNewTableView ();
	connection->setTailFile(fname);
	connection->m_session_state.m_csv_separator = separator;
	
	m_main_window->statusBar()->showMessage(tr("Tail!"));
	connection->handleCSVSetup(fname);
	connection->processTailCSVStream();
	emit newConnection(connection);
}

void Server::createTailDataStream (QString const & fname)
{
	Connection * connection = createNewTableView ();
	connection->setTailFile(fname);

	m_main_window->statusBar()->showMessage(tr("Tail!"));
	connection->handleCSVSetup(fname);
	connection->processTailCSVStream();
	emit newConnection(connection);
}

void Server::incomingConnection (qintptr socketDescriptor)
{
	Connection * connection = createNewTableView ();
	connection->setSocketDescriptor(socketDescriptor);

	QObject::connect(connection->m_tcpstream, SIGNAL(readyRead()), connection, SLOT(processReadyRead()));
	QObject::connect(connection->m_tcpstream, SIGNAL(disconnected()), connection, SLOT(onDisconnected()));
	m_main_window->statusBar()->showMessage(tr("Incomming tcp connection!"));
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
		it->second->onShowPlots();
}
void Server::onHidePlots ()
{
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		it->second->onHidePlots();
}
void Server::onShowTables ()
{
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		it->second->onShowTables();
}
void Server::onHideTables ()
{
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		it->second->onHideTables();
}

// @TODO: hmm. this whole fn is.. unfortunately rushed. need to rethink
void Server::onClickedAtDockedWidgets (QModelIndex idx)
{
	TreeModel * model = static_cast<TreeModel *>(m_main_window->getDockedWidgetsTreeView()->model());

	QList<QString> path;
	QList<bool> state;
	path.push_front(model->data(idx).toString());
	state.push_front(model->data(idx, Qt::CheckStateRole).toInt());
	QModelIndex parent = model->parent(idx);
	while (parent.isValid())
	{
		path.push_front(model->data(parent).toString());
		state.push_front(model->data(parent, Qt::CheckStateRole).toInt());
		parent = model->parent(parent);
	}

	//path[0]=WarHorse_App path[1]=table path[2]=pokus path[3]=--
	qDebug("path[0]=%s", path.size() > 0 ? path.at(0).toStdString().c_str() : "--");
	qDebug("path[1]=%s", path.size() > 1 ? path.at(1).toStdString().c_str() : "--");
	qDebug("path[2]=%s", path.size() > 2 ? path.at(2).toStdString().c_str() : "--");
	qDebug("path[3]=%s", path.size() > 3 ? path.at(3).toStdString().c_str() : "--");

	Q_ASSERT(path.size());

	if (Connection * conn = findConnectionByName(path.at(0)))
	{
		if (path.size() > 1)
		{
			QString class_type = path.at(1);
			if (class_type == "table")
			{
				//path.pop_front(); // drop app name
				//path.pop_front(); // drop widget identifier

				if (path.size() > 2)
				{
					for (datatables_t::iterator it = conn->m_data.get<e_data_table>().begin(), ite = conn->m_data.get<e_data_table>().end(); it != ite; ++it)
					{
						DataTable * dp = (*it);
						if (dp->m_config.m_tag == path.at(2))
						{
							bool apply = false;
							bool const xchg = dp->widget().getConfig().m_show ^ state.at(2);
							apply |= xchg;
							if (xchg)
							{
								dp->m_config.m_show = state.at(2);
							}

							if (state.at(2))
								dp->onShow();
							else
								dp->onHide();

							if (apply)
								dp->widget().applyConfig(dp->widget().getConfig());
						}
					}
				}
				else
				{
					if (state.at(1))
						conn->onShowTables();
					else
						conn->onHideTables();
				}
			}
			else
			{
				if (path.size() > 2)
				{
					for (dataplots_t::iterator it = conn->m_data.get<e_data_plot>().begin(), ite = conn->m_data.get<e_data_plot>().end(); it != ite; ++it)
					{
						DataPlot * dp = (*it);
						if (dp->m_config.m_tag == path.at(2))
						{
							bool apply = false;
							bool const xchg = dp->widget().getConfig().m_show ^ state.at(2);
							apply |= xchg;
							if (xchg)
							{
								dp->m_config.m_show = state.at(2);
							}

							if (path.size() > 3)
							{
								for (int cc = 0, cce = dp->m_config.m_ccfg.size(); cc < cce; ++cc)
								{
									plot::CurveConfig & cfg = dp->m_config.m_ccfg[cc];
									if (cfg.m_tag == path.at(3))
									{
										apply |= cfg.m_show ^ state.at(3);
										cfg.m_show = state.at(3);
										break;
									}
								}
							}
							else if (path.size() > 2)
							{
								for (int cc = 0, cce = dp->m_config.m_ccfg.size(); cc < cce; ++cc)
								{
									plot::CurveConfig & cfg = dp->m_config.m_ccfg[cc];
									apply |= cfg.m_show ^ state.at(2);
									cfg.m_show = state.at(2);
								}

								if (state.at(2))
									dp->onShow();
								else
									dp->onHide();
							}

							if (apply)
							{
								dp->widget().applyConfig(dp->widget().getConfig());
							}
						}
					}
				}
				else
				{
					if (state.at(1))
						conn->onShowPlots();
					else
						conn->onHidePlots();
				}
			}
		}

	}


}

void Server::onCloseTab (QWidget * w)
{
	if (w)
	{
		int const idx = m_main_window->getTabTrace()->indexOf(w);
		if (idx != -1)
		{
			qDebug("Server::onCloseTab(QWidget *) idx=%i", idx);
			onCloseTab(idx, w);
		}
	}
}
void Server::onCloseTabWithIndex (int idx)
{
	if (idx != -1)
	{
		if (QWidget * w = m_main_window->getTabTrace()->widget(idx))
		{
			qDebug("Server::onCloseTabWithIndex(int) idx=%i widget=0x%08x", idx, w);
			onCloseTab(idx, w);
		}
	}
}
void Server::onCloseCurrentTab ()
{
	qDebug("%s", __FUNCTION__);
	QWidget * w = m_main_window->getTabTrace()->currentWidget();
	onCloseTab(w);
}

void Server::destroyConnection (Connection * connection)
{
	QObject::disconnect(connection->m_tcpstream, SIGNAL(disconnected()), connection, SLOT(onDisconnected()));
	QObject::disconnect(connection->m_tcpstream, SIGNAL(readyRead()), connection, SLOT(processReadyRead()));
	QObject::disconnect(connection->m_table_view_widget->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));

	delete connection;
}

void Server::onCloseTab (int idx, QWidget * w)
{
	qDebug("Server::onCloseTab(idx=%i, QWidget *=0x%08x) idx=%i", idx, w, idx);
	m_main_window->getTabTrace()->removeTab(idx);
	connections_t::iterator it = m_connections.find(w);
	if (it != m_connections.end())
	{
		Connection * connection = it->second;
		m_connections.erase(it);
		destroyConnection(connection);
	}
	qDebug("Server::onCloseTab(idx=%i, QWidget *=0x%08x) curr idx=%i", idx, w, m_main_window->getTabTrace()->currentIndex());
	onTabTraceFocus(m_main_window->getTabTrace()->currentIndex());
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

/*profiler::ProfilerWindow * Server::createNewProfilerView ()
{
	return 0;
}

void Server::incomingProfilerConnection (profiler::profiler_rvp_t * rvp)
{

	using namespace profiler;
	ProfilerWindow * w = new ProfilerWindow(m_main_window, this, rvp);
	qDebug("Incoming profiler rendez-vous point!");
	m_main_window->statusBar()->showMessage(tr("Incoming profiler rendez-vous point!"));
}*/

