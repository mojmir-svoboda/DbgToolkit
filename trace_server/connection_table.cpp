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

DataTable::DataTable (Connection * connection, QString const & confname, QStringList const & path)
	: DockedData<e_data_table>(connection, confname, path)
{
	qDebug("%s this=0x%08x name=%s", __FUNCTION__, this, confname.toStdString().c_str());
	m_widget = new table::TableWidget(connection, 0, m_config, confname, path);
	m_widget->setItemDelegate(new SyncedTableItemDelegate(m_widget));
}

DataTable::~DataTable ()
{
	//QObject::disconnect(m_widget->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));
}

void Connection::onShowTables ()
{
	qDebug("%s", __FUNCTION__);
	/*for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		(*it)->onShow();
		m_main_window->restoreDockWidget((*it)->m_wd);
	}*/
}

void Connection::onHideTables ()
{
	qDebug("%s", __FUNCTION__);
	/*for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		(*it)->onHide();
	}*/
}

void Connection::onShowTableContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		(*it)->widget().onHideContextMenu();
	}
}

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
		else if (cmd.m_tvs[i].m_tag == tlv::tag_time)
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
		else if (cmd.m_tvs[i].m_tag == tlv::tag_time)
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
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetTableTag, tag);
	qDebug("table: load cfg file=%s", fname.toStdString().c_str());
	return loadConfig(config, fname);
}

bool Connection::loadConfigForTables (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		DataTable * const tbl = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetTableTag, tbl->m_config.m_tag);
		loadConfig(tbl->m_config, fname);
		tbl->widget().applyConfig(tbl->m_config);
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
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetTableTag, tag);
	qDebug("table save cfg file=%s", fname.toStdString().c_str());
	return saveConfig(config, fname);
}

bool Connection::saveConfigForTables (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		DataTable * const tbl = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetTableTag, tbl->m_config.m_tag);
		tbl->widget().onSaveButton();
	}
	return true;
}
*/

datatables_t::iterator Connection::findOrCreateTable (QString const & tag)
{
	datatables_t::iterator it = dataWidgetFactory<e_data_table>(tag);
	if (it != m_data.get<e_data_table>().end())
	{
		// TMP!
/*		(*it)->m_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
		(*it)->m_widget->verticalHeader()->setFont(m_main_window->tableFont());*/
		//(*it)->m_widget->verticalHeader()->setDefaultSectionSize(m_main_window->tableRowSize());
		(*it)->m_widget->verticalHeader()->setDefaultSectionSize(16);
		(*it)->m_widget->verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance
		(*it)->m_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
		(*it)->m_widget->setSelectionMode(QAbstractItemView::SingleSelection);
		QObject::connect((*it)->widget().horizontalHeader(), SIGNAL(sectionResized(int, int, int)), &(*it)->widget(), SLOT(onSectionResized(int, int, int)));

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

	DataTable & dp = **it;
	dp.widget().appendTableXY(x, y, time, fgc, bgc, subtag);
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

	DataTable & dp = **it;
	dp.widget().appendTableSetup(x, y, time, fgc, bgc, hhdr, subtag);
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
		//	(*it)->widget().clearCurveData(subtag);
		//}
	}
	return true;
}
void Connection::requestTableSynchronization (int sync_group, unsigned long long time)
{
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		DataTable * const tbl = *it;
		if (tbl->widget().getConfig().m_sync_group == sync_group)
			tbl->widget().findNearestTimeRow(time);
	}
}

void Connection::requestTableWheelEventSync (int sync_group, QWheelEvent * ev, QTableView const * source)
{
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		DataTable * const tbl = *it;
		if (tbl->widget().getConfig().m_sync_group == sync_group)
			tbl->widget().requestTableWheelEventSync(ev, source);

		/*int const hmin = tbl->widget().horizontalScrollBar()->minimum();
		int const hval = tbl->widget().horizontalScrollBar()->value();
		int const hmax = tbl->widget().horizontalScrollBar()->maximum();
		int const vmin = tbl->widget().verticalScrollBar()->minimum();
		int const vval = tbl->widget().verticalScrollBar()->value();
		int const vmax = tbl->widget().verticalScrollBar()->maximum();
		qDebug("conn wh sync: min=%i val=%i max=%i", vmin, vval, vmax);*/
	}
}

void Connection::requestTableActionSync (int sync_group, unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QTableView const * source)
{
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		DataTable * const tbl = *it;
		if (tbl->widget().getConfig().m_sync_group == sync_group)
			tbl->widget().requestTableActionSync(t, cursorAction, modifiers, source);
	}
}
