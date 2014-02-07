#include "mainwindow.h"
#include "connection.h"
#include <QClipboard>
#include <QStatusBar>

Connection * MainWindow::findCurrentConnection ()
{
	Q_ASSERT(parent());
	QWidget * w = getTabTrace()->currentWidget();
	connections_t::iterator it = m_connections.find(w);
	return (it != m_connections.end()) ? it->second : 0;
}

Connection * MainWindow::findConnectionByName (QString const & app_name)
{
	Q_ASSERT(parent());

	for (connections_t::const_iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		if (it->second->getAppName() == app_name)
			return it->second;
	return 0;
}

void MainWindow::copyStorageTo (QString const & filename)
{
	if (Connection * conn = findCurrentConnection())
		conn->copyStorageTo(filename);
}

void MainWindow::onLevelValueChanged (int val)
{
	qDebug("level changed: %u", val);
	if (Connection * conn = findCurrentConnection())
		conn->onLevelValueChanged(val);
}

/*void MainWindow::onCopyToClipboard ()
{
	if (Connection * conn = findCurrentConnection())
	{
		QString selection = conn->onCopyToClipboard();
        qApp->clipboard()->setText(selection);
	}
}*/

void MainWindow::onBufferingStateChanged (int state)
{
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		it->second->onBufferingStateChanged(state);
}



void MainWindow::onTabTraceFocus (int i)
{
	QWidget * w = getTabTrace()->widget(i);
	// @TODO: unfocus current
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
	{
		if (it->second->m_tab_widget == w)
		{
			it->second->onTabTraceFocus();
			return;
		}
	}
}

void MainWindow::newConnection (Connection * c)
{
	statusBar()->showMessage(tr("Incomming tcp connection!"));
}

Connection * MainWindow::createNewConnection ()
{
	// toto (cely?) do mainwindow
	// emit createNewApplicationTab ();
	Connection * connection = new Connection(this);
	QWidget * tab = new QWidget();
	connection->m_tab_widget = tab;

	int const n = getTabTrace()->addTab(tab, QString::fromUtf8("???"));
	qDebug("created new tab at %u for connection @ 0x%08x", n, connection);

	m_connections.insert(std::make_pair(tab, connection));
	return connection;
}

void MainWindow::importDataStream (QString const & fname)
{
	QFile file(fname);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning(QString(tr("Could not open file %1").arg(fname)).toLatin1());
		return;
	}

	Connection * connection = createNewConnection();
	connection->setImportFile(fname);
	statusBar()->showMessage(tr("Importing file!"));

	QDataStream import_stream(&file);
	connection->processDataStream(import_stream);
	file.close();
}

void MainWindow::createTailLogStream (QString const & fname, QString const & separator)
{
	Connection * connection = createNewConnection();
	connection->setTailFile(fname);
	//connection->m_csv_separator = separator;
	
	statusBar()->showMessage(tr("Tail!"));
	connection->handleCSVSetup(fname);
	connection->processTailCSVStream();
	emit newConnection(connection);
}

void MainWindow::createTailDataStream (QString const & fname)
{
	Connection * connection = createNewConnection();
	connection->setTailFile(fname);

	statusBar()->showMessage(tr("Tail!"));
	connection->handleCSVSetup(fname);
	connection->processTailCSVStream();
	emit newConnection(connection);
}


void MainWindow::onCloseTab (int idx, QWidget * w)
{
	qDebug("MainWindow::onCloseTab(idx=%i, QWidget *=0x%08x) idx=%i", idx, w, idx);
	getTabTrace()->removeTab(idx);
	connections_t::iterator it = m_connections.find(w);
	if (it != m_connections.end())
	{
		Connection * connection = it->second;
		m_connections.erase(it);
		destroyConnection(connection);
	}
	qDebug("MainWindow::onCloseTab(idx=%i, QWidget *=0x%08x) curr idx=%i", idx, w, getTabTrace()->currentIndex());
	onTabTraceFocus(getTabTrace()->currentIndex());
}
void MainWindow::onCloseMarkedTabs ()
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

void MainWindow::onCloseTab (QWidget * w)
{
	if (w)
	{
		int const idx = getTabTrace()->indexOf(w);
		if (idx != -1)
		{
			qDebug("MainWindow::onCloseTab(QWidget *) idx=%i", idx);
			onCloseTab(idx, w);
		}
	}
}
void MainWindow::onCloseTabWithIndex (int idx)
{
	if (idx != -1)
	{
		if (QWidget * w = getTabTrace()->widget(idx))
		{
			qDebug("MainWindow::onCloseTabWithIndex(int) idx=%i widget=0x%08x", idx, w);
			onCloseTab(idx, w);
		}
	}
}
void MainWindow::onCloseCurrentTab ()
{
	qDebug("%s", __FUNCTION__);
	QWidget * w = getTabTrace()->currentWidget();
	onCloseTab(w);
}

void MainWindow::destroyConnection (Connection * connection)
{
	QObject::disconnect(connection->m_tcpstream, SIGNAL(disconnected()), connection, SLOT(onDisconnected()));
	QObject::disconnect(connection->m_tcpstream, SIGNAL(readyRead()), connection, SLOT(processReadyRead()));
	//QObject::disconnect(connection->m_table_view_widget->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));

	delete connection;
}




