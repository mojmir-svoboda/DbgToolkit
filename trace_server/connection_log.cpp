#include "connection.h"
#include <QClipboard>
#include <QObject>
#include "logs/logtablemodel.h"
#include "logs/logwidget.h"
#include "tableview.h"
#include "constants.h"
#include "utils.h"

DataLog::DataLog (Connection * connection, config_t & config, QString const & confname, QStringList const & path)
	: DockedData<e_data_log>(connection, config, confname, path)
{
	qDebug("%s this=0x%08x name=%s", __FUNCTION__, this, confname.toStdString().c_str());

	QWidget * tab = connection->m_tab_widget;
	QHBoxLayout * horizontalLayout = new QHBoxLayout(tab);
	horizontalLayout->setSpacing(1);
	horizontalLayout->setContentsMargins(0, 0, 0, 0);
	horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
	logs::LogWidget * tableView = new logs::LogWidget(connection, tab, this->config(), confname);
	horizontalLayout->addWidget(tableView);
	m_widget = tableView;
}

DataLog::~DataLog ()
{
	//QObject::disconnect(connection->m_table_view_widget->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));
}

bool Connection::handleLogCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	QString const tag("default"); // @FIXME
	//int const slash_pos = tag.lastIndexOf(QChar('/'));
	//tag.chop(msg_tag.size() - slash_pos);

	//QString subtag = msg_tag;
	//subtag.remove(0, slash_pos + 1);

	datalogs_t::iterator it = findOrCreateLog(tag);

	DataLog & dp = **it;

	dp.widget().handleCommand(cmd, mode);

	//m_main_window->getWidgetFile()->hideLinearParents();
	return true;
}


bool Connection::handleLogClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	return true;
}

void Connection::onShowLogs ()
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
	/*for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		(*it)->onHide();
	}*/
}

void Connection::onShowLogContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		(*it)->widget().onHideContextMenu();
	}
}

/*bool Connection::loadConfigForLogs (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		DataLog * const tbl = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetLogTag, tbl->m_config.m_tag);
		loadConfig(tbl->m_config, fname);
		tbl->widget().applyConfig(tbl->m_config);
		if (tbl->m_config.m_show)
			tbl->onShow();
		else
			tbl->onHide();
	}
	return true;
}*/

/*bool Connection::saveConfigForLog (logs::LogConfig const & config, QString const & tag)
{
	QString const preset_name = m_curr_preset.isEmpty() ? m_main_window->getValidCurrentPresetName() : m_curr_preset;
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetLogTag, tag);
	qDebug("log save cfg file=%s", fname.toStdString().c_str());
	return saveConfig(config, fname);
}

bool Connection::saveConfigForLogs (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		DataLog * const tbl = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetLogTag, tbl->m_config.m_tag);
		tbl->widget().onSaveButton();
	}
	return true;
}*/

	/*Connection * conn = server->findConnectionByName(app_name);
	if (conn)
	{
		qDebug("cmd setup: looking for app=%s: not found", app_name.toStdString().c_str());
		if (!m_main_window->clrFltEnabled())
		{
			m_file_model->beforeLoad();
			loadSessionState(conn->sessionState(), m_session_state);
		}

		QWidget * w = conn->m_tab_widget;
		server->onCloseTab(w);	// close old one
		// @TODO: delete persistent storage for the tab

		m_file_model->afterLoad();
	}
	else
	{
		qDebug("cmd setup: looking for app=%s: found", app_name.toStdString().c_str());
		m_file_model->beforeLoad();
		QString const pname = m_main_window->matchClosestPresetName(app_name);
		m_main_window->onPresetActivate(this, pname);
		m_file_model->afterLoad();
	}*/

datalogs_t::iterator Connection::findOrCreateLog (QString const & tag)
{
	datalogs_t::iterator it = dataWidgetFactory<e_data_log>(tag);
	if (it != m_data.get<e_data_log>().end())
	{
		DataLog * d = *it;
		//d->onShow();
	}
	else
		assert(false);
	return it;
}




void Connection::onTabTraceFocus ()
{
	if (!m_curr_preset.isEmpty())
		m_main_window->setPresetAsCurrent(m_curr_preset);

	// @TODO: set app level to spinbox
	// @TODO: set app buffering to spinbox

	//m_main_window->setLastSearchIntoCombobox(m_last_search);
}


