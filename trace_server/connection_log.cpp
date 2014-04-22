#include "connection.h"
#include <QClipboard>
#include <QObject>
#include "logs/logtablemodel.h"
#include "logs/logwidget.h"
#include "tableview.h"
#include "constants.h"
#include "utils.h"

datalogs_t::iterator Connection::findOrCreateLog (QString const & tag)
{
	datalogs_t::iterator it = m_data.get<e_data_log>().find(tag);
	if (it == m_data.get<e_data_log>().end())
	{
		it = dataWidgetFactory<e_data_log>(tag);
		(*it)->widget().setupNewLogModel();
		(*it)->widget().applyConfig(); // 0 means "create new model"
	}
	return it;
}

// @FIXME: dodelat!!!!!!!!
bool Connection::handleLogCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	//QString const tag(g_MainLogName); // @FIXME
	QString const tag("messages"); // @FIXME
	//int const slash_pos = tag.lastIndexOf(QChar('/'));
	//tag.chop(msg_tag.size() - slash_pos);
	//QString subtag = msg_tag;
	//subtag.remove(0, slash_pos + 1);

	datalogs_t::iterator it = findOrCreateLog(tag);
	DataLog & dp = **it;

	dp.widget().handleCommand(cmd, mode);
	return true;
}

bool Connection::handleLogClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	return true;
}

/*void Connection::onShowLogs ()
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		//(*it)->onShow();
		m_main_window->restoreDockWidget((*it)->m_wd);
	}
}

void Connection::onHideLogs ()
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		(*it)->onHide();
	}
}

void Connection::onShowLogContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		(*it)->widget().onHideContextMenu();
	}
}*/


