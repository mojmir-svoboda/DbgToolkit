#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QAbstractSocket>
#include <QTcpServer>
#include <QModelIndex>

class Connection;
class QTcpServer;
class QNetworkSession;
class QFile;

class Server : public QTcpServer
{
	Q_OBJECT
public:
	static unsigned short const default_port = 13127;
	explicit Server (QObject *parent = 0);

	QString const & getStatus () const { return status; }
	void incomingDataStream (QDataStream & stream);
	void exportStorageTo (QString const & filename);
	
signals:
	void newConnection (Connection * connection);

public slots:
	void onDoubleClickedAtFileTree (QModelIndex idx);
	void onSectionResized (int logicalIndex, int oldSize, int newSize);
	void onEditingFinished ();
	void onApplyFilterClicked ();
	void onLevelValueChanged (int val);

protected:
	void incomingConnection (int socketDescriptor);
	Connection * createNewTableView ();
	
private:
	QString status;
	std::map<int, Connection *> connections;
};

#endif // SERVER_H
