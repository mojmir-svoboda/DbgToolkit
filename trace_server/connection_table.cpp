#include "connection.h"
#include <QtNetwork>
#include <QHeaderView>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"
#include "cmd.h"
#include "utils.h"
#include "dock.h"
#include <cstdlib>

void Connection::onShowTables ()
{
	qDebug("%s", __FUNCTION__);
	for (datatables_t::iterator it = m_datatables.begin(), ite = m_datatables.end(); it != ite; ++it)
	{
		(*it)->onShow();
		m_main_window->restoreDockWidget((*it)->m_wd);
	}
	//m_main_window->onTableRestoreButton();
}

void Connection::onHideTables ()
{
	qDebug("%s", __FUNCTION__);
	for (datatables_t::iterator it = m_datatables.begin(), ite = m_datatables.end(); it != ite; ++it)
	{
		(*it)->onHide();
	}
}

void Connection::onShowTableContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (datatables_t::iterator it = m_datatables.begin(), ite = m_datatables.end(); it != ite; ++it)
	{
		(*it)->getWidget().onHideContextMenu();
	}
}

bool Connection::handleTableXYCommand (DecodedCommand const & cmd)
{
	std::string tag;
	int x = 0;
	int y = 0;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_msg)
			tag = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_x)
			x = atoi(cmd.tvs[i].m_val.c_str());
		else if (cmd.tvs[i].m_tag == tlv::tag_y)
			y = atoi(cmd.tvs[i].m_val.c_str());
	}

	appendTableXY(x, y, QString::fromStdString(tag));
	return true;
}

bool Connection::loadConfigForTable (table::TableConfig & config, QString const & tag)
{
	QString const fname = getDataTagFileName(getConfig().m_appdir, sessionState().m_name, "table", tag);
	qDebug("load tag file=%s", fname.toStdString().c_str());
	return loadConfig(config, fname);
}

bool Connection::saveConfigForTable (table::TableConfig const & config, QString const & tag)
{
	QString const fname = getDataTagFileName(getConfig().m_appdir, sessionState().m_name, "table", tag);
	qDebug("save tag file=%s", fname.toStdString().c_str());
	return saveConfig(config, fname);
}

void Connection::appendTableXY (int x, int y, QString const & msg_tag)
{
	QString tag = msg_tag;
	int const slash_pos = tag.lastIndexOf(QChar('/'));
	tag.chop(msg_tag.size() - slash_pos);

	QString subtag = msg_tag;
	subtag.remove(0, slash_pos + 1);
	QString const table_name = sessionState().m_name + "/table/" + tag;

	datatables_t::iterator it = m_datatables.find(tag);
	if (it == m_datatables.end())
	{
		// new data table
		table::TableConfig template_config;
		template_config.m_tag = tag;
		QString const fname = getDataTagFileName(getConfig().m_appdir, sessionState().m_name, "table", tag);
		if (loadConfigForTable(template_config, tag))
		{
			qDebug("table: loaded tag configuration from file=%s", fname.toStdString().c_str());
		}
		
		DataTable * const dp = new DataTable(this, template_config, fname);
		it = m_datatables.insert(tag, dp);
		QModelIndex const item_idx = m_data_model->insertItem(table_name.toStdString());

		QObject::connect(dp->m_table.horizontalHeader(), SIGNAL(sectionResized(int, int, int)), &dp->m_table, SLOT(onSectionResized(int, int, int)));
		dp->m_wd = m_main_window->m_dock_mgr.mkDockWidget(m_main_window, &dp->m_table, table_name);
		if (m_main_window->tableEnabled())
		{
			if (template_config.m_show)
			{
				dp->m_table.show();
				dp->m_wd->show();
				m_main_window->restoreDockWidget(dp->m_wd);
				m_main_window->onPlotRestoreButton();
			}
		}
		else
		{
			dp->m_table.hide();
			dp->m_wd->hide();
		}
	}

	DataTable & dp = **it;
	dp.m_table.appendTableXY(x, y, subtag);
}

