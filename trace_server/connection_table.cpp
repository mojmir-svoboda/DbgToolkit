#include "connection.h"
#include <QtNetwork>
#include <QHeaderView>
#include <QScrollBar>
#include <tlv_parser/tlv_encoder.h>
#include "cmd.h"
#include "utils.h"
#include "dock.h"
#include "delegates.h"
#include <cstdlib>

bool Connection::handleTableXYCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	QString tag;
	QString time;
	int x = 0;
	int y = 0;
	QString fgc;
	QString bgc;
	for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i)
	{
		if (cmd.m_tvs[i].m_tag == tlv::tag_msg)
			tag = cmd.m_tvs[i].m_val;
		else if (cmd.m_tvs[i].m_tag == tlv::tag_x)
			x = cmd.m_tvs[i].m_val.toInt();
		else if (cmd.m_tvs[i].m_tag == tlv::tag_y)
			y = cmd.m_tvs[i].m_val.toInt();
		else if (cmd.m_tvs[i].m_tag == tlv::tag_ctime)
			time = cmd.m_tvs[i].m_val;
		else if (cmd.m_tvs[i].m_tag == tlv::tag_fgc)
			fgc = cmd.m_tvs[i].m_val;
		else if (cmd.m_tvs[i].m_tag == tlv::tag_bgc)
			bgc = cmd.m_tvs[i].m_val;
	}

	if (m_main_window->tableState() != e_FtrDisabled)
	{
		appendTableXY(x, y, time, fgc, bgc, tag);
	}
	return true;
}

bool Connection::handleTableSetupCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	QString tag;
	QString time;
	int x = 0;
	int y = 0;
	QString fgc, bgc;
	QString hhdr;
	for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i)
	{
		if (cmd.m_tvs[i].m_tag == tlv::tag_msg)
			tag = cmd.m_tvs[i].m_val;
		else if (cmd.m_tvs[i].m_tag == tlv::tag_x)
			x = cmd.m_tvs[i].m_val.toInt();
		else if (cmd.m_tvs[i].m_tag == tlv::tag_y)
			y = cmd.m_tvs[i].m_val.toInt();
		else if (cmd.m_tvs[i].m_tag == tlv::tag_ctime)
			time = cmd.m_tvs[i].m_val;
		else if (cmd.m_tvs[i].m_tag == tlv::tag_hhdr)
			hhdr = cmd.m_tvs[i].m_val;
		else if (cmd.m_tvs[i].m_tag == tlv::tag_fgc)
			fgc = cmd.m_tvs[i].m_val;
		else if (cmd.m_tvs[i].m_tag == tlv::tag_bgc)
			bgc = cmd.m_tvs[i].m_val;
	}

	//qDebug("table: setup hdr: x=%i y=%i hhdr=%s fg=%s bg=%s", x, y, hhdr.toStdString().c_str(), fgc.toStdString().c_str(), bgc.toStdString().c_str());

	if (m_main_window->tableState() != e_FtrDisabled)
		appendTableSetup(x, y, time, fgc, bgc, hhdr, tag);
	return true;
}


/*bool Connection::loadConfigForTable (QString const & preset_name, table::TableConfig & config, QString const & tag)
{
	QString const fname = getDataTagFileName(config().m_appdir, preset_name, g_presetTableTag, tag);
	qDebug("table: load cfg file=%s", fname.toStdString().c_str());
	return loadConfig(config, fname);
}

bool Connection::loadConfigForTables (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		DataTable * const tbl = *it;
		QString const fname = getDataTagFileName(config().m_appdir, preset_name, g_presetTableTag, tbl->m_config.m_tag);
		loadConfig(tbl->m_config, fname);
		tbl->applyConfig(tbl->m_config);
		if (tbl->m_config.m_show)
			tbl->onShow();
		else
			tbl->onHide();
	}
	return true;
}*/

/*
bool Connection::saveConfigForTable (table::TableConfig const & config, QString const & tag)
{
	QString const preset_name = m_curr_preset.isEmpty() ? m_main_window->getValidCurrentPresetName() : m_curr_preset;
	QString const fname = getDataTagFileName(config().m_appdir, preset_name, g_presetTableTag, tag);
	qDebug("table save cfg file=%s", fname.toStdString().c_str());
	return saveConfig(config, fname);
}

bool Connection::saveConfigForTables (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		DataTable * const tbl = *it;
		QString const fname = getDataTagFileName(config().m_appdir, preset_name, g_presetTableTag, tbl->m_config.m_tag);
		tbl->onSaveButton();
	}
	return true;
}
*/

datatables_t::iterator Connection::findOrCreateTable (QString const & tag)
{
	typedef SelectIterator<e_data_table>::type iterator;
	iterator it = m_data.get<e_data_table>().find(tag);
	if (it == m_data.get<e_data_table>().end())
	{
		it = dataWidgetFactory<e_data_table>(tag);
		//(*it)->setupNewLogModel();
		(*it)->applyConfig(); // 0 means "create new model"

		// TMP!
/*		(*it)->m_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
		(*it)->m_widget->verticalHeader()->setFont(m_main_window->tableFont());*/
		//(*it)->m_widget->verticalHeader()->setDefaultSectionSize(m_main_window->tableRowSize());
		(*it)->verticalHeader()->setDefaultSectionSize(16);
		(*it)->verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance
		(*it)->setSelectionBehavior(QAbstractItemView::SelectRows);
		(*it)->setSelectionMode(QAbstractItemView::SingleSelection);
		QObject::connect((*it)->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), *it, SLOT(onSectionResized(int, int, int)));

		if (m_main_window->tableState() == e_FtrEnabled && (*it)->config().m_show)
		{
			//(*it)->show();
		}
		else
		{
			//(*it)->hide();
		}
	}
	return it;
}

void Connection::appendTableXY (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & msg_tag)
{
	QString tag = msg_tag;
	int const slash_pos = tag.lastIndexOf(QChar('/'));
	tag.chop(msg_tag.size() - slash_pos);

	QString subtag = msg_tag;
	subtag.remove(0, slash_pos + 1);

	datatables_t::iterator it = findOrCreateTable(tag);

	(*it)->appendTableXY(x, y, time, fgc, bgc, subtag);
}

void Connection::appendTableSetup (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & hhdr, QString const & msg_tag)
{
	QString tag = msg_tag;
	QString subtag = msg_tag;
	int const slash_pos = tag.lastIndexOf(QChar('/'));
	if (slash_pos >= 0)
	{
		tag.chop(msg_tag.size() - slash_pos);
		subtag.remove(0, slash_pos + 1);
	}

	datatables_t::iterator it = findOrCreateTable(tag);
	(*it)->appendTableSetup(x, y, time, fgc, bgc, hhdr, subtag);
}

bool Connection::handleTableClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	QString msg;
	for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i)
	{
		if (cmd.m_tvs[i].m_tag == tlv::tag_msg)
			msg = cmd.m_tvs[i].m_val;
	}

	if (m_main_window->plotState() != e_FtrDisabled)
	{
		QString tag = msg;
		int const slash_pos = tag.lastIndexOf(QChar('/'));
		tag.chop(msg.size() - slash_pos);

		QString subtag = msg;
		subtag.remove(0, slash_pos + 1);

		//dataplots_t::iterator it = m_dataplots.find(tag);
		//if (it != m_dataplots.end())
		//{
		//	(*it)->clearCurveData(subtag);
		//}
	}
	return true;
}

//@TODO: old call!
void Connection::requestTableSynchronization (int sync_group, unsigned long long time)
{
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		if ((*it)->config().m_sync_group == sync_group)
			(*it)->findNearestTimeRow(time);
	}
}

void Connection::requestTableWheelEventSync (int sync_group, QWheelEvent * ev, QTableView const * source)
{
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		if ((*it)->config().m_sync_group == sync_group)
			(*it)->requestTableWheelEventSync(ev, source);

		/*int const hmin = (*it)->horizontalScrollBar()->minimum();
		int const hval = (*it)->horizontalScrollBar()->value();
		int const hmax = (*it)->horizontalScrollBar()->maximum();
		int const vmin = (*it)->verticalScrollBar()->minimum();
		int const vval = (*it)->verticalScrollBar()->value();
		int const vmax = (*it)->verticalScrollBar()->maximum();
		qDebug("conn wh sync: min=%i val=%i max=%i", vmin, vval, vmax);*/
	}
}

void Connection::requestTableActionSync (int sync_group, unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QTableView const * source)
{
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		if ((*it)->config().m_sync_group == sync_group)
			(*it)->requestTableActionSync(t, cursorAction, modifiers, source);
	}
}
