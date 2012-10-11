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
}

void Connection::onHideTables ()
{
	qDebug("%s", __FUNCTION__);
}

void Connection::onShowTableContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
}

bool Connection::handleTableCommand (DecodedCommand const & cmd)
{
	std::string tag;
	double x = 0.0;
	double y = 0.0;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_msg)
			tag = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_x)
			x = atof(cmd.tvs[i].m_val.c_str());
		else if (cmd.tvs[i].m_tag == tlv::tag_y)
			y = atof(cmd.tvs[i].m_val.c_str());
	}

	appendTableXY(QString::fromStdString(tag), x, y);
	return true;
}

bool Connection::loadConfigForTable (table::TableConfig & config, QString const & tag)
{
	QString const fname = getDataTagFileName(getConfig().m_appdir, sessionState().m_name, tag);
	qDebug("load tag file=%s", fname.toStdString().c_str());
	return loadConfig(config, fname);
}

bool Connection::saveConfigForTable (table::TableConfig const & config, QString const & tag)
{
	QString const fname = getDataTagFileName(getConfig().m_appdir, sessionState().m_name, tag);
	qDebug("save tag file=%s", fname.toStdString().c_str());
	return saveConfig(config, fname);
}

void Connection::appendTableXY (QString const & msg_tag, double x, double y)
{
	QString tag = msg_tag;
	int const slash_pos = tag.lastIndexOf(QChar('/'));
	tag.chop(msg_tag.size() - slash_pos);

	QString subtag = msg_tag;
	subtag.remove(0, slash_pos + 1);
	QString const table_name = sessionState().m_name + "/" + tag;

	datatables_t::iterator it = m_datatables.find(tag);
	if (it == m_datatables.end())
	{
		// new data table
		table::TableConfig template_config;
		template_config.m_tag = tag;
		QString const fname = getDataTagFileName(getConfig().m_appdir, sessionState().m_name, tag);
		if (loadConfigForTable(template_config, tag))
		{
			qDebug("table: loaded tag configuration from file=%s", fname.toStdString().c_str());
		}
		
		DataPlot * const dp = new DataPlot(this, template_config, fname);
		it = m_datatables.insert(tag, dp);
		QModelIndex const item_idx = m_tables_model->insertItem(table_name.toStdString());

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

	DataPlot & dp = **it;
	QString const curve_name = table_name + "/" + subtag;
	table::Curve * curve = dp.m_table.findCurve(subtag);

	if (!curve)
	{
		curve = *dp.m_table.mkCurve(subtag);
		QModelIndex const item_idx = m_tables_model->insertItem(curve_name.toStdString());

		table::CurveConfig const * ccfg = 0;
		dp.m_config.findCurveConfig(subtag, ccfg); // config is created by mkCurve

		if (dp.m_config.m_show)
		{
			bool const visible = ccfg ? ccfg->m_show : true;
			dp.m_table.showCurve(curve->m_curve, visible);
			m_tables_model->setData(item_idx, QVariant(visible ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
		}
		else
		{
			bool const visible = ccfg ? ccfg->m_show : false;
			dp.m_table.showCurve(curve->m_curve, visible);
			m_tables_model->setData(item_idx, QVariant(visible ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
		}
	}





	// if (autoscroll && need_to) shift m_from;
}

