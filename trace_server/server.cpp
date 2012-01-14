#include <QtGui>
#include <QtNetwork/QtNetwork>
#include <stdlib.h>
#include "server.h"
#include "connection.h"
#include "mainwindow.h"
#include "modelview.h"

void SaveTableViewSettings (QTableView * tb)
{
	QSettings settings("MojoMir", "TraceServer");
	QByteArray state = tb->horizontalHeader()->saveState();
	settings.setValue("column_width", state);
}

Server::Server(QObject *parent)
	: QTcpServer(parent)
{
	QHostAddress addr(QHostAddress::LocalHost);
	if (!listen(addr, default_port)) {
		status = tr("Unable to start server! Reason: %1").arg(errorString());
		QTimer::singleShot(3000, qApp, SLOT(quit()));
		return;
	}

	status = tr("Server running at IP: %1 port: %2").arg(serverAddress().toString()).arg(serverPort());
}

void Server::exportStorageTo (QString const & filename)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	int const n = main_window->getTabTrace()->currentIndex();
	if (n >= 0)
	{
		connections[n]->exportStorageTo(filename);
	}
}

void Server::onSectionResized (int idx, int oldSize, int newSize)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	int const n = main_window->getTabTrace()->currentIndex();
	if (n >= 0)
	{
		QString const & name = connections[n]->m_name;
		if (name.length())
		{
			size_t app_idx = main_window->findAppName(name);
			if (idx < main_window->getColumnSizes(app_idx).size())
			{
				qDebug("onSectionResized idx=%i size: %i->%i", idx, oldSize, newSize);
				main_window->getColumnSizes(app_idx)[idx] = newSize;
			}
		}
	}
}

void Server::onApplyFilterClicked ()
{
	qDebug("apply button!");
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	int const n = main_window->getTabTrace()->currentIndex();
	if (n >= 0)
		connections[n]->onApplyFilterClicked();
}

void Server::onLevelValueChanged (int val)
{
	qDebug("level changed!");
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	int const n = main_window->getTabTrace()->currentIndex();
	if (n >= 0)
		connections[n]->onLevelValueChanged(val);
}

void Server::onEditingFinished ()
{
}

void Server::onDoubleClickedAtFileTree (QModelIndex idx)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	std::vector<QString> s;
	s.reserve(16);
	QStandardItemModel * model = static_cast<QStandardItemModel *>(main_window->getTreeViewFile()->model());
	QStandardItem * item = model->itemFromIndex(idx);
	QStandardItem * line_item = 0;
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

	qDebug("file=%s", file.c_str());

	bool const checked = (item->checkState() == Qt::Checked);
	Connection::fileline_t filter_item(file, std::string());
	if (line_item)
	{
		QString const & val = model->data(idx, Qt::DisplayRole).toString();
		filter_item.second = val.toStdString();
	}

	qDebug("dbl click! (checked=%u) %s : %s", checked, filter_item.first.c_str(), filter_item.second.c_str());
	int const n = main_window->getTabTrace()->currentIndex();

	if (n >= 0)
	{
		if (!checked)
			connections[n]->appendFileFilter(filter_item);
		else
			connections[n]->removeFileFilter(filter_item);
	}
	item->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
}

Connection * Server::createNewTableView ()
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	Connection * connection = new Connection(this);
	connection->setMainWindow(main_window);
	connection->setupModelFile();
	QWidget * tab = new QWidget();
	QHBoxLayout * horizontalLayout = new QHBoxLayout(tab);
	horizontalLayout->setSpacing(6);
	horizontalLayout->setContentsMargins(11, 11, 11, 11);
	horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
	QTableView * tableView = new QTableView(tab);
	tableView->setObjectName(QString::fromUtf8("tableView"));
	ModelView * model = new ModelView(tableView, connection);
	tableView->verticalHeader()->setDefaultSectionSize(12);
	tableView->setModel(model);
	horizontalLayout->addWidget(tableView);
	connection->setTableViewWidget(tableView);
	connection->setupThreadColors(main_window->getThreadColors());
	int const n = main_window->getTabTrace()->addTab(tab, QString::fromUtf8("???"));
	connection->setTabWidget(n);
	main_window->getTabTrace()->setCurrentIndex(n);
	connections.insert(std::make_pair(n, connection));
	QObject::connect(main_window->getTabTrace(), SIGNAL(currentChanged(int)), connection, SLOT(onTabTraceFocus(int)));
	QObject::connect(tableView->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));
	return connection;
}

void Server::incomingDataStream (QDataStream & stream)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	Connection * connection = createNewTableView ();
	qDebug("new data stream in tab=%i", connection->m_tab_idx);
	main_window->statusBar()->showMessage(tr("Importing file!"));
	connection->processDataStream(stream);
}

void Server::incomingConnection (int socketDescriptor)
{
	MainWindow * main_window = static_cast<MainWindow *>(parent());
	Connection * connection = createNewTableView ();
	connection->setSocketDescriptor(socketDescriptor);
	QObject::connect(connection, SIGNAL(readyRead()), connection, SLOT(processReadyRead()));
	QObject::connect(connection, SIGNAL(disconnected()), connection, SLOT(onDisconnected()));
	qDebug("new connection in tab=%i", connection->m_tab_idx);
	main_window->statusBar()->showMessage(tr("Incomming connection!"));
	emit newConnection(connection);
}


