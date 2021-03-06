/**
 * Copyright (C) 2011 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once
#include <QObject>
#include <QAbstractSocket>
#include <QTcpServer>

class Connection;
class QTcpServer;
class QNetworkSession;
class MainWindow;

class Server : public QTcpServer
{
	Q_OBJECT
public:
	explicit Server (QString addr, unsigned short port, QObject * parent = 0, bool no_quit_msg_box = false);

	QString const & getRunningStatus () const { return m_running_server_status; }
	void stop () { close(); }

signals:
	void newConnection (Connection * connection);
	void statusChanged (QString const & status);

public slots:
	void incomingConnection (qintptr socketDescriptor);
	
private:
	friend class MainWindow;
	MainWindow * m_main_window;
	QString m_running_server_status;
};
