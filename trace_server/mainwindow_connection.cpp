#include "mainwindow.h"
#include "connection.h"
#include <QClipboard>
#include <QStatusBar>
#include <QTimer>
#include "setupdialogcsv.h"
#include "ui_setupdialogcsv.h"

Connection * MainWindow::findConnectionByName (QString const & app_name)
{
	Q_ASSERT(parent());

	for (connections_t::const_iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		if ((*it)->getAppName() == app_name)
			return (*it);
	return 0;
}

void MainWindow::newConnection (Connection * c)
{
	statusBar()->showMessage(tr("Incomming tcp connection!"));
}

Connection * MainWindow::createNewConnection ()
{
	static int conn_counter = 0;
	QString const tmp_name = QString("new_app_") + QString::number(++conn_counter);
	Connection * connection = new Connection(tmp_name, this);
	m_connections.push_back(connection);
	qDebug("created new connection[%u] for connection @ 0x%08x", m_connections.size(), connection);
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
	QFileInfo fi(fname);
	QString const tag = fi.fileName();
	
	connection->handleCSVSetup(tag);
	datalogs_t::iterator it = connection->findOrCreateLog(tag);
	connection->m_config.m_csv_separator = separator;

	connection->processTailCSVStream();
	emit newConnection(connection);
}

void MainWindow::onChangeSetupDialogCSV (int n)
{
}

void MainWindow::createTailDataStream (QString const & fname)
{
	Connection * connection = createNewConnection();
	connection->setTailFile(fname);
	QFileInfo fi(fname);
	QString const tag = fi.fileName();

	enum { e_min_lines = 3 };
	QString lines[e_min_lines];
	int n_lines = 0;
	while (!connection->m_file_csv_stream->atEnd() && n_lines < e_min_lines)
	{
		lines[n_lines] = connection->m_file_csv_stream->readLine(2048);
		++n_lines;
	}

	m_setup_dialog_csv->m_data = lines;
	connect(m_setup_dialog_csv->ui->separatorComboBox, SIGNAL(activated(int)), this, SLOT(onChangeSetupDialogCSV(int)));
	connect(m_setup_dialog_csv->ui->headerCheckBox, SIGNAL(activated(int)), this, SLOT(onChangeSetupDialogCSV(int)));

	m_setup_dialog_csv->exec();

	connection->m_file_csv_stream->seek(0);
	
	connection->handleCSVSetup(tag);
	datalogs_t::iterator it = connection->findOrCreateLog(tag);
	//(*it)->config().m_csv_separator = separator;

	connection->processTailCSVStream();
	emit newConnection(connection);
}

void MainWindow::onCloseConnection (Connection * c)
{
	qDebug("MainWindow::onCloseConection(Connection *=0x%08x)", c);
	connections_t::iterator it = std::find(m_connections.begin(), m_connections.end(), c);
	if (it != m_connections.end())
	{
		Connection * connection = (*it);
		m_connections.erase(it);
		delete connection;
		connection = 0;
	}
}

void MainWindow::onCloseMarkedConnections ()
{
	qDebug("%s", __FUNCTION__);

	std::vector<Connection *> to_delete;
	to_delete.reserve(m_connections.size());

	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
	{
		if ((*it)->m_marked_for_close)
			to_delete.push_back(*it);
	}

	while (!to_delete.empty())
	{
		onCloseConnection(to_delete.back());
		to_delete.pop_back();
	}
}

void MainWindow::markConnectionForClose (Connection * conn)
{
	conn->m_marked_for_close = true;
	QTimer::singleShot(0, this, SLOT(onCloseMarkedConnections()));
}
