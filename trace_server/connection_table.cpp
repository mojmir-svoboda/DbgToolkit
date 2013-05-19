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

DataTable::DataTable (Connection * connection, config_t & config, QString const & fname)
	: DockedData<e_data_table>(connection, config, fname)
{
	qDebug("%s this=0x%08x name=%s", __FUNCTION__, this, fname.toStdString().c_str());
	m_widget = new table::TableWidget(connection, 0, m_config, fname);
	m_widget->setItemDelegate(new SyncedTableItemDelegate(m_widget));
}

void Connection::onShowTables ()
{
	qDebug("%s", __FUNCTION__);
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		(*it)->onShow();
		m_main_window->restoreDockWidget((*it)->m_wd);
	}
}

void Connection::onHideTables ()
{
	qDebug("%s", __FUNCTION__);
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		(*it)->onHide();
	}
}

void Connection::onShowTableContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (datatables_t::iterator it = m_data.get<e_data_table>().begin(), ite = m_data.get<e_data_table>().end(); it != ite; ++it)
	{
		(*it)->widget().onHideContextMenu();
	}
}

bool Connection::handleTableXYCommand (DecodedCommand const & cmd)
{
	QString tag;
	QString time;
	int x = 0;
	int y = 0;
	QString fgc;
	QString bgc;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_msg)
			tag = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_x)
			x = cmd.tvs[i].m_val.toInt();
		else if (cmd.tvs[i].m_tag == tlv::tag_y)
			y = cmd.tvs[i].m_val.toInt();
		else if (cmd.tvs[i].m_tag == tlv::tag_time)
			time = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_fgc)
			fgc = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_bgc)
			bgc = cmd.tvs[i].m_val;
	}

	if (m_main_window->tableState() != e_FtrDisabled)
	{
		appendTableXY(x, y, time, fgc, bgc, tag);
	}
	return true;
}

bool Connection::handleTableSetupCommand (DecodedCommand const & cmd)
{
	QString tag;
	QString time;
	int x = 0;
	int y = 0;
	QString fgc, bgc;
	QString hhdr;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_msg)
			tag = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_x)
			x = cmd.tvs[i].m_val.toInt();
		else if (cmd.tvs[i].m_tag == tlv::tag_y)
			y = cmd.tvs[i].m_val.toInt();
		else if (cmd.tvs[i].m_tag == tlv::tag_time)
			time = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_hhdr)
			hhdr = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_fgc)
			fgc = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_bgc)
			bgc = cmd.tvs[i].m_val;
	}

	//qDebug("table: setup hdr: x=%i y=%i hhdr=%s fg=%s bg=%s", x, y, hhdr.toStdString().c_str(), fgc.toStdString().c_str(), bgc.toStdString().c_str());

	if (m_main_window->tableState() != e_FtrDisabled)
		appendTableSetup(x, y, time, fgc, bgc, hhdr, tag);
	return true;
}


bool Connection::loadConfigForTable (QString const & preset_name, table::TableConfig & config, QString const & tag)
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
}

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


datatables_t::iterator Connection::findOrCreateTable (QString const & tag)
{
	QString const table_name = sessionState().getAppName() + "/" + g_presetTableTag + "/" + tag;

	datatables_t::iterator it = m_data.get<e_data_table>().find(tag);
	if (it == m_data.get<e_data_table>().end())
	{
		qDebug("table: creating table %s", tag.toStdString().c_str());
		// new data table
		table::TableConfig template_config;
		template_config.m_tag = tag;

		QString const preset_name = m_main_window->matchClosestPresetName(sessionState().getAppName());
		QString fname;
		if (!preset_name.isEmpty())
		{
			fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetTableTag, tag);
			loadConfigForTable(preset_name, template_config, tag);
		}
		
		DataTable * const dp = new DataTable(this, template_config, fname);
		it = m_data.get<e_data_table>().insert(tag, dp);
		QModelIndex const item_idx = m_data_model->insertItemWithHint(table_name, template_config.m_show);

		// TMP!
/*		dp->m_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
		dp->m_widget->verticalHeader()->setFont(m_main_window->tableFont());*/
		//dp->m_widget->verticalHeader()->setDefaultSectionSize(m_main_window->tableRowSize());
		dp->m_widget->verticalHeader()->setDefaultSectionSize(16);
		dp->m_widget->verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance
		dp->m_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
		dp->m_widget->setSelectionMode(QAbstractItemView::SingleSelection);

		QObject::connect(dp->widget().horizontalHeader(), SIGNAL(sectionResized(int, int, int)), &dp->widget(), SLOT(onSectionResized(int, int, int)));
		dp->m_wd = m_main_window->m_dock_mgr.mkDockWidget(m_main_window, &dp->widget(), template_config.m_show, table_name);
		bool const visible = template_config.m_show;
		m_data_model->setData(item_idx, QVariant(visible ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
		if (m_main_window->tableState() == e_FtrEnabled && visible)
		{
			m_main_window->loadLayout(preset_name);
			dp->onShow();
		}
		else
		{
			dp->onHide();
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

bool Connection::handleTableClearCommand (DecodedCommand const & cmd)
{
	QString msg;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_msg)
			msg = cmd.tvs[i].m_val;
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
