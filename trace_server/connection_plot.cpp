#include "connection.h"
#include <QtNetwork>
#include <QHeaderView>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"
#include "cmd.h"
#include "utils.h"
#include "dock.h"
#include <cstdlib>

DataPlot::DataPlot (Connection * parent, plot::PlotConfig & config, QString const & fname)
	: m_parent(parent)
	, m_wd(0)
	, m_config(config)
	, m_plot(0)
	, m_from(0)
	, m_fname(fname)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	m_plot = new plot::BasePlot(parent, 0, m_config, fname);
}
DataPlot::~DataPlot ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	delete m_plot;
	m_plot = 0;
}
void DataPlot::onShow ()
{
	m_wd->show();
	m_plot->onShow();
}
void DataPlot::onHide ()
{
	//m_wd->hide();
	m_plot->onHide();
	QTimer::singleShot(0, m_wd, SLOT(hide()));
}


void Connection::onShowPlots ()
{
	qDebug("%s", __FUNCTION__);
	for (dataplots_t::iterator it = m_dataplots.begin(), ite = m_dataplots.end(); it != ite; ++it)
	{
		(*it)->onShow();
		m_main_window->restoreDockWidget((*it)->m_wd);
	}
	m_main_window->onDockRestoreButton();
}

void Connection::onHidePlots ()
{
	qDebug("%s", __FUNCTION__);
	for (dataplots_t::iterator it = m_dataplots.begin(), ite = m_dataplots.end(); it != ite; ++it)
	{
		(*it)->onHide();
	}
}

void Connection::onShowPlotContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (dataplots_t::iterator it = m_dataplots.begin(), ite = m_dataplots.end(); it != ite; ++it)
	{
		(*it)->widget().onHideContextMenu();
	}
}

bool Connection::handleDataXYCommand (DecodedCommand const & cmd)
{
	QString tag;
	double x = 0.0;
	double y = 0.0;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_msg)
			tag = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_x)
			x = cmd.tvs[i].m_val.toDouble();
		else if (cmd.tvs[i].m_tag == tlv::tag_y)
			y = cmd.tvs[i].m_val.toDouble();
	}

	if (m_main_window->plotEnabled())
		appendDataXY(x, y, tag);
	return true;
}

bool Connection::handleDataXYZCommand (DecodedCommand const & cmd)
{
	return true;
}
 
bool Connection::loadConfigForPlots (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (dataplots_t::iterator it = m_dataplots.begin(), ite = m_dataplots.end(); it != ite; ++it)
	{
		DataPlot * const plt = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetPlotTag, plt->m_config.m_tag);
		loadConfig(plt->m_config, fname);
		plt->widget().applyConfig(plt->m_config);
		if (plt->m_config.m_show)
			plt->onShow();
		else
			plt->onHide();
	}
	return true;
}

bool Connection::loadConfigForPlot (QString const & preset_name, plot::PlotConfig & config, QString const & tag)
{
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetPlotTag, tag);
	qDebug("load tag file=%s", fname.toStdString().c_str());

	return loadConfig(config, fname);
}

bool Connection::saveConfigForPlot (plot::PlotConfig const & config, QString const & tag)
{
	QString const preset_name = m_curr_preset.isEmpty() ? m_main_window->getCurrentPresetName() : m_curr_preset;
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetPlotTag, tag);
	qDebug("save tag file=%s", fname.toStdString().c_str());

	return saveConfig(config, fname);
}

bool Connection::saveConfigForPlots (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (dataplots_t::iterator it = m_dataplots.begin(), ite = m_dataplots.end(); it != ite; ++it)
	{
		DataPlot * const plt = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetPlotTag, plt->m_config.m_tag);
		saveConfig(plt->m_config, fname);
	}
	return true;
}

void Connection::appendDataXY (double x, double y, QString const & msg_tag)
{
	QString tag = msg_tag;
	int const slash_pos = tag.lastIndexOf(QChar('/'));
	tag.chop(msg_tag.size() - slash_pos);

	QString subtag = msg_tag;
	subtag.remove(0, slash_pos + 1);
	QString const plot_name = sessionState().getAppName() + "/plot/" + tag;

	dataplots_t::iterator it = m_dataplots.find(tag);
	if (it == m_dataplots.end())
	{
		// new data plot
		plot::PlotConfig template_config;
		template_config.m_tag = tag;
		QString const preset_name = m_curr_preset.isEmpty() ? m_main_window->getValidCurrentPresetName() : m_curr_preset;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetPlotTag, tag);
		if (!preset_name.isEmpty())
			if (loadConfigForPlot(preset_name, template_config, tag))
			{
				qDebug("plot: loaded tag configuration from file=%s", fname.toStdString().c_str());
			}
		
		DataPlot * const dp = new DataPlot(this, template_config, fname);
		it = m_dataplots.insert(tag, dp);
		QModelIndex const item_idx = m_data_model->insertItemWithHint(plot_name, template_config.m_show);

		dp->m_wd = m_main_window->m_dock_mgr.mkDockWidget(m_main_window, &dp->widget(), template_config.m_show, plot_name);
		if (m_main_window->plotEnabled() && template_config.m_show)
		{
			dp->onShow();
			m_main_window->onDockRestoreButton();
		}
		else
		{
			dp->onHide();
		m_main_window->onDockRestoreButton();
		}
	}

	DataPlot & dp = **it;
	QString const curve_name = plot_name + "/" + subtag;
	plot::Curve * curve = dp.widget().findCurve(subtag);

	if (!curve)
	{
		curve = *dp.widget().mkCurve(subtag);

		plot::CurveConfig const * ccfg = 0;
		dp.m_config.findCurveConfig(subtag, ccfg); // config is created by mkCurve

		QModelIndex const item_idx = m_data_model->insertItemWithHint(curve_name, ccfg ? ccfg->m_show : true);

		if (dp.m_config.m_show)
		{
			bool const visible = ccfg ? ccfg->m_show : true;
			dp.widget().showCurve(curve->m_curve, visible);
			m_data_model->setData(item_idx, QVariant(visible ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
		}
		else
		{
			bool const visible = ccfg ? ccfg->m_show : false;
			dp.widget().showCurve(curve->m_curve, visible);
			m_data_model->setData(item_idx, QVariant(visible ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
		}
	}

	curve->m_data->push_back(x, y);

	// if (autoscroll && need_to) shift m_from;
}

