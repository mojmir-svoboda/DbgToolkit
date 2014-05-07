#include "connection.h"
#include <QtNetwork>
#include <QHeaderView>
#include <QScrollBar>
#include <tlv_parser/tlv_encoder.h>
#include "cmd.h"
#include "utils.h"
#include "dock.h"
#include "delegates.h"
#include "mainwindow.h"
#include <cstdlib>

bool Connection::handleTableXYCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	QString tag;
	if (!cmd.getString(tlv::tag_msg, tag)) return true;
	QString ctime;
	if (!cmd.getString(tlv::tag_ctime, ctime)) return true;
	int x = 0;
	if (!cmd.get(tlv::tag_ix, x)) return true;
	int y = 0;
	if (!cmd.get(tlv::tag_iy, y)) return true;
	QString fgc;
	cmd.getString(tlv::tag_fgc, fgc);
	QString bgc;
	cmd.getString(tlv::tag_bgc, bgc);

	if (getClosestFeatureState(e_data_table) != e_FtrDisabled)
	{
		appendTableXY(x, y, ctime, fgc, bgc, tag);
	}
	return true;
}

bool Connection::handleTableSetupCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	QString tag;
	if (!cmd.getString(tlv::tag_msg, tag)) return true;
	QString ctime;
	if (!cmd.getString(tlv::tag_ctime, ctime)) return true;
	int x = 0;
	if (!cmd.get(tlv::tag_ix, x)) return true;
	int y = 0;
	if (!cmd.get(tlv::tag_iy, y)) return true;
	QString fgc;
	cmd.getString(tlv::tag_fgc, fgc);
	QString bgc;
	cmd.getString(tlv::tag_bgc, bgc);
	QString hhdr;
	cmd.getString(tlv::tag_hhdr, bgc);

	if (getClosestFeatureState(e_data_table) != e_FtrDisabled)
	{
    //qDebug("table: setup hdr: x=%i y=%i hhdr=%s fg=%s bg=%s", x, y, hhdr.toStdString().c_str(), fgc.toStdString().c_str(), bgc.toStdString().c_str());
		appendTableSetup(x, y, ctime, fgc, bgc, hhdr, tag);
	}
	return true;
}

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

		if (getClosestFeatureState(e_data_table) == e_FtrEnabled && (*it)->config().m_show)
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
	if (!cmd.getString(tlv::tag_msg, msg)) return true;

	if (getClosestFeatureState(e_data_table) != e_FtrDisabled)
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
