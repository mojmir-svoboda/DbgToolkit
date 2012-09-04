#include "connection.h"
#include <QtNetwork>
#include <QHeaderView>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"
#include "cmd.h"
#include "utils.h"
#include "dock.h"
#include <cstdlib>

void Connection::onShowPlots ()
{
	qDebug("%s", __FUNCTION__);
	for (dataplots_t::iterator it = m_dataplots.begin(), ite = m_dataplots.end(); it != ite; ++it)
	{
		(*it)->onShowPlots();
	}
}

void Connection::onHidePlots ()
{
	qDebug("%s", __FUNCTION__);
	for (dataplots_t::iterator it = m_dataplots.begin(), ite = m_dataplots.end(); it != ite; ++it)
	{
		(*it)->onHidePlots();
	}
}

void Connection::onShowPlotContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (dataplots_t::iterator it = m_dataplots.begin(), ite = m_dataplots.end(); it != ite; ++it)
	{
		(*it)->m_plot.onHidePlotContextMenu();
	}
}

bool Connection::handleDataXYCommand (DecodedCommand const & cmd)
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

	appendDataXY(QString::fromStdString(tag), x, y);
	return true;
}

bool Connection::handleDataXYZCommand (DecodedCommand const & cmd)
{
	return true;
}
 
bool Connection::loadConfigForPlot (plot::PlotConfig & config, QString const & tag)
{
	QString const fname = getDataTagFileName(getConfig().m_appdir, sessionState().m_name, tag);
	qDebug("load tag file=%s", fname.toStdString().c_str());

	return loadConfig(config, fname);
}

bool Connection::saveConfigForPlot (plot::PlotConfig const & config, QString const & tag)
{
	QString const fname = getDataTagFileName(getConfig().m_appdir, sessionState().m_name, tag);
	qDebug("save tag file=%s", fname.toStdString().c_str());

	return saveConfig(config, fname);
}

void Connection::appendDataXY (QString const & msg_tag, double x, double y)
{
	QString tag = msg_tag;
	int const slash_pos = tag.lastIndexOf(QChar('/'));
	tag.chop(msg_tag.size() - slash_pos);

	QString subtag = msg_tag;
	subtag.remove(0, slash_pos + 1);
	//qDebug("consumed node: 0x%016x, tis_sz=%u bis_sz=%u b=%u block_msg=%s", node, tis.size(), bis.size(), b, block.m_msg.c_str());

	dataplots_t::iterator it = m_dataplots.find(tag);
	if (it == m_dataplots.end())
	{
		// new data plot
		plot::PlotConfig template_config;
		QString const fname = getDataTagFileName(getConfig().m_appdir, sessionState().m_name, tag);
		if (loadConfigForPlot(template_config, tag))
		{
			qDebug("plot: loaded tag configuration from file=%s", fname.toStdString().c_str());
		}
		
		DataPlot * const dp = new DataPlot(this, template_config, fname);
		it = m_dataplots.insert(tag, dp);
		QString const plots_name = sessionState().m_name + "/" + msg_tag;
		QModelIndex const item_idx = m_plots_model->insertItem(plots_name.toStdString());

		QString const complete_name = sessionState().m_name + "/" + tag;
		dp->m_wd = m_main_window->m_dock_mgr.mkDockWidget(m_main_window, &dp->m_plot, complete_name);
		plot::Curve * curve = (*it)->m_plot.findCurve(subtag);
		plot::CurveConfig const * ccfg = 0;
		dp->m_config.findCurveConfig(subtag, ccfg);
	
		if (template_config.m_show)
		{
			bool const visible = ccfg ? ccfg->m_show : true;
			dp->m_plot.showCurve(curve->m_curve, visible);
			m_plots_model->setData(item_idx, QVariant(visible ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
		}
		else
		{
			bool const visible = ccfg ? ccfg->m_show : false;
			dp->m_plot.showCurve(curve->m_curve, visible);
			m_plots_model->setData(item_idx, QVariant(visible ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
		}

		if (m_main_window->plotEnabled())
		{
			dp->m_plot.show();
		}
		else
		{
			dp->m_plot.hide();
			dp->m_wd->hide();
		}
		m_main_window->onPlotRestoreButton();
	}
	else
	{
		(*it)->m_plot.findCurve(subtag)->m_data->push_back(x, y);
	}

	// if (autoscroll && need_to) shift m_from;
}

