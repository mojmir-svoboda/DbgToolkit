#include "connection.h"
#include <QtNetwork>
#include <QHeaderView>
#include <tlv_parser/tlv_encoder.h>
#include "cmd.h"
#include "utils.h"
#include "dock.h"
#include <cstdlib>

DataPlot::DataPlot (Connection * connection, QString const & confname, QStringList const & path)
	: DockedData<e_data_plot>(connection, confname, path)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	m_widget = new plot::PlotWidget(connection, 0, m_config, confname, path);
}

void Connection::onShowPlots ()
{
	qDebug("%s", __FUNCTION__);
	/*for (dataplots_t::iterator it = m_data.get<e_data_plot>().begin(), ite = m_data.get<e_data_plot>().end(); it != ite; ++it)
	{
		(*it)->onShow();
		m_main_window->restoreDockWidget((*it)->m_wd);
	}*/
}

void Connection::onHidePlots ()
{
	qDebug("%s", __FUNCTION__);
	/*for (dataplots_t::iterator it = m_data.get<e_data_plot>().begin(), ite = m_data.get<e_data_plot>().end(); it != ite; ++it)
	{
		(*it)->onHide();
	}*/
}

void Connection::onShowPlotContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (dataplots_t::iterator it = m_data.get<e_data_plot>().begin(), ite = m_data.get<e_data_plot>().end(); it != ite; ++it)
	{
		(*it)->widget().onHideContextMenu();
	}
}

bool Connection::handlePlotCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	if (m_main_window->plotState() == e_FtrDisabled)
		return true;

	QString tag;
	for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i) // @TODO: precache
	{
		if (cmd.m_tvs[i].m_tag == tlv::tag_msg)
			tag = cmd.m_tvs[i].m_val;
	}

	int const slash_pos = tag.lastIndexOf(QChar('/'));
	tag.chop(tag.size() - slash_pos);

	//QString subtag = msg_tag;
	//subtag.remove(0, slash_pos + 1);

	dataplots_t::iterator it = findOrCreatePlot(tag);
	DataPlot & dp = **it;

	dp.widget().handleCommand(cmd, mode);
	return true;
}
/*	QString tag;
	double x = 0.0;
	double y = 0.0;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i) // @TODO: precache
	{
		if (cmd.tvs[i].m_tag == tlv::tag_msg)
			tag = cmd.tvs[i].m_val;
	}

	int const slash_pos = tag.lastIndexOf(QChar('/'));
	tag.chop(msg_tag.size() - slash_pos);

	QString subtag = msg_tag;
	subtag.remove(0, slash_pos + 1);


	dataplots_t::iterator it = findOrCreatePlot(tag);
	DataPlot & dp = **it;

	dp.widget().handleDataXYCommand(cmd, mode);
}*/

bool Connection::handleDataXYZCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	return true;
}
 
dataplots_t::iterator Connection::findOrCreatePlot (QString const & tag)
{
	typedef SelectIterator<e_data_plot>::type iterator;
	iterator it = m_data.get<e_data_plot>().find(tag);
	if (it == m_data.get<e_data_plot>().end())
	{
		it = dataWidgetFactory<e_data_plot>(tag);
		(*it)->widget().applyConfig();
	}
	return it;
}

