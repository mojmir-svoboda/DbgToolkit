#include "connection.h"
#include <QtNetwork>
#include <QHeaderView>
#include <QScrollBar>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"
#include "cmd.h"
#include "utils.h"
#include "dock.h"
#include <cstdlib>

DataTable::DataTable (Connection * parent, table::TableConfig & config, QString const & fname)
	: m_parent(parent)
	, m_wd(0)
	, m_config(config)
	, m_table(0)
	, m_fname(fname)
{
	qDebug("%s this=0x%08x name=%s", __FUNCTION__, this, fname.toStdString().c_str());
	m_table = new table::BaseTable(parent, 0, m_config, fname);
}
DataTable::~DataTable ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	delete m_table;
	m_table = 0;
}
void DataTable::onShow ()
{
	m_wd->show();
	m_table->onShow();
}
void DataTable::onHide ()
{
	m_wd->hide();
	m_table->onHide();
}

void Connection::onShowTables ()
{
	qDebug("%s", __FUNCTION__);
	for (datatables_t::iterator it = m_datatables.begin(), ite = m_datatables.end(); it != ite; ++it)
	{
		(*it)->onShow();
		m_main_window->restoreDockWidget((*it)->m_wd);
	}
	m_main_window->onDockRestoreButton();
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

	if (m_main_window->tableEnabled())
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

	if (m_main_window->tableEnabled())
		appendTableSetup(x, y, time, fgc, bgc, hhdr, tag);
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

datatables_t::iterator Connection::findOrCreateTable (QString const & tag)
{
	QString const table_name = sessionState().m_name + "/table/" + tag;

	datatables_t::iterator it = m_datatables.find(tag);
	if (it == m_datatables.end())
	{
		qDebug("table: creating table %s", tag.toStdString().c_str());
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
		QModelIndex const item_idx = m_data_model->insertItem(table_name);

		// TMP!
/*		dp->m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
		dp->m_table->verticalHeader()->setFont(m_main_window->tableFont());*/
		//dp->m_table->verticalHeader()->setDefaultSectionSize(m_main_window->tableRowSize());
		dp->m_table->verticalHeader()->setDefaultSectionSize(16);
		dp->m_table->verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance
		dp->m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
		dp->m_table->setSelectionMode(QAbstractItemView::SingleSelection);

		QObject::connect(dp->widget().horizontalHeader(), SIGNAL(sectionResized(int, int, int)), &dp->widget(), SLOT(onSectionResized(int, int, int)));
		dp->m_wd = m_main_window->m_dock_mgr.mkDockWidget(m_main_window, &dp->widget(), table_name);
		if (m_main_window->tableEnabled())
		{
			if (template_config.m_show)
			{
				dp->onShow();
			}
		}
		else
		{
			dp->onHide();
		}
		m_main_window->onDockRestoreButton();
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

void Connection::requestTableSynchronization (int sync_group, unsigned long long time)
{
	for (datatables_t::iterator it = m_datatables.begin(), ite = m_datatables.end(); it != ite; ++it)
	{
		DataTable * const tbl = *it;
		if (tbl->widget().getConfig().m_sync_group == sync_group)
			tbl->widget().findNearestTimeRow(time);
	}
}

void Connection::requestTableWheelEventSync (int sync_group, QWheelEvent * ev, QTableView const * source)
{
	for (datatables_t::iterator it = m_datatables.begin(), ite = m_datatables.end(); it != ite; ++it)
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
	for (datatables_t::iterator it = m_datatables.begin(), ite = m_datatables.end(); it != ite; ++it)
	{
		DataTable * const tbl = *it;
		if (tbl->widget().getConfig().m_sync_group == sync_group)
			tbl->widget().requestTableActionSync(t, cursorAction, modifiers, source);
	}
}
