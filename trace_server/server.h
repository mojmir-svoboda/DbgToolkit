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
#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QAbstractSocket>
#include <QTcpServer>
#include <QModelIndex>
#include "rvps.h"
#include "profilerwindow.h"

class Connection;
class QTcpServer;
class QNetworkSession;
class QFile;
class MainWindow;

class Server : public QTcpServer
{
	Q_OBJECT
public:
	static unsigned short const default_port = 13127;
	explicit Server (QString addr, unsigned short port, QObject * parent = 0, bool quit_delay = true);

	QString const & getStatus () const { return status; }
	void incomingDataStream (QDataStream & stream);
	void copyStorageTo (QString const & filename);
	void exportStorageToCSV (QString const & filename);
	Connection * findConnectionByName (QString const & app_name);
	
signals:
	void newConnection (Connection * connection);

public slots:
	void onClickedAtCtxTree (QModelIndex idx);
	void onDoubleClickedAtCtxTree (QModelIndex idx);

	void onClickedAtTIDList (QModelIndex idx);
	void onDoubleClickedAtTIDList (QModelIndex idx);

	void onClickedAtLvlList (QModelIndex idx);
	void onDoubleClickedAtLvlList (QModelIndex idx);

	void onClickedAtRegexList (QModelIndex idx);
	void onDoubleClickedAtRegexList (QModelIndex idx);

	void onClickedAtColorRegexList (QModelIndex idx);
	void onDoubleClickedAtColorRegexList (QModelIndex idx);

	void onClickedAtStringList (QModelIndex idx);
	void onDoubleClickedAtStringList (QModelIndex idx);

	void onSectionResized (int logicalIndex, int oldSize, int newSize);
	void onEditingFinished ();
	void onCopyToClipboard ();

	void onHidePrevFromRow ();
	void onUnhidePrevFromRow ();
	void onExcludeFileLine ();
	void onToggleRefFromRow ();
	void onClearCurrentView ();
	void onClearCurrentFileFilter ();
	void onClearCurrentCtxFilter ();
	void onClearCurrentTIDFilter ();
	void onClearCurrentColorizedRegexFilter ();
	void onClearCurrentRegexFilter ();
	void onClearCurrentStringFilter ();
	void onClearCurrentScopeFilter ();
	void onClearCurrentRefTime ();
	void onApplyColumnSetup ();

	void onShowPlots ();
	void onClickedAtPlotTree (QModelIndex idx);
	void onHidePlots ();
	void onShowTables ();
	void onHideTables ();

	void onCloseMarkedTabs ();
	void onCloseCurrentTab ();
	void onCloseTab (QWidget * w);
	void onCloseTab (int idx, QWidget * w);
	void onCloseTabWithIndex (int);

	void onTabTraceFocus (int i);

	void destroyConnection (Connection * c);

	void onFilterFile (int state);
	void onLevelValueChanged (int val);
	void onBufferingStateChanged (int state);

	profiler::ProfilerWindow * createNewProfilerView ();
	void incomingProfilerConnection (profiler::profiler_rvp_t * rvp);

protected:
	friend class MainWindow;
	void incomingConnection (int socketDescriptor);
	Connection * createNewTableView ();
	Connection * findCurrentConnection ();
	
private:
	QString status;
	typedef std::map<QWidget *, Connection *> connections_t;
	connections_t m_connections;
};

#endif // SERVER_H
