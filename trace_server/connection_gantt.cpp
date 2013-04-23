#include "connection.h"
#include "constants.h"
#include <tlv_parser/tlv_encoder.h>
#include "cmd.h"
#include "utils.h"
#include "dock.h"
#include "delegates.h"
#include "ganttview.h"
//#include <cstdlib>

DataGantt::DataGantt (Connection * parent, gantt::GanttConfig & config, QString const & fname)
	: m_parent(parent)
	, m_wd(0)
	, m_config(config)
	, m_widget(0)
	, m_fname(fname)
{
	qDebug("%s this=0x%08x name=%s", __FUNCTION__, this, fname.toStdString().c_str());
	m_widget = new gantt::BaseGantt(parent, 0, m_config, fname);
	//m_widget->setItemDelegate(new SyncedGanttItemDelegate(m_widget));
}
DataGantt::~DataGantt ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	delete m_widget;
	m_widget = 0;
}
void DataGantt::onShow ()
{
	m_widget->onShow();
	m_wd->show();
	m_parent->getMainWindow()->restoreDockWidget(m_wd);
	//QTimer::singleShot(0, m_parent, SLOT(onShowGantts()));
}
void DataGantt::onHide ()
{
	m_widget->onHide();
	QTimer::singleShot(0, m_wd, SLOT(hide()));
}

void Connection::onShowGantts ()
{
	qDebug("%s", __FUNCTION__);
	for (datagantts_t::iterator it = m_datagantts.begin(), ite = m_datagantts.end(); it != ite; ++it)
	{
		(*it)->onShow();
		m_main_window->restoreDockWidget((*it)->m_wd);
	}
}

void Connection::onHideGantts ()
{
	qDebug("%s", __FUNCTION__);
	for (datagantts_t::iterator it = m_datagantts.begin(), ite = m_datagantts.end(); it != ite; ++it)
	{
		(*it)->onHide();
	}
}

void Connection::onShowGanttContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (datagantts_t::iterator it = m_datagantts.begin(), ite = m_datagantts.end(); it != ite; ++it)
	{
		(*it)->widget().onHideContextMenu();
	}
}

bool parseCommand (DecodedCommand const & cmd, gantt::DecodedData & dd)
{
	QString msg;
	QString tid;
	QString time;
	QString fgc;
	QString bgc;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_msg)
			msg = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_time)
			time = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_tid)
			tid = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_fgc)
			fgc = cmd.tvs[i].m_val;
		else if (cmd.tvs[i].m_tag == tlv::tag_bgc)
			bgc = cmd.tvs[i].m_val;
	}

	QString subtag = msg;
	int const slash_pos0 = subtag.lastIndexOf(QChar('/'));
	subtag.chop(msg.size() - slash_pos0);

	QString tag = subtag;
	int const slash_pos1 = tag.lastIndexOf(QChar('/'));
	tag.chop(tag.size() - slash_pos1);

	subtag.remove(0, slash_pos1 + 1);
	msg.remove(0, slash_pos0 + 1);

	//if (!subtag.contains("Dude"))
	//	return false;

	dd.m_time = time.toULongLong();
	dd.m_ctx = tid.toULongLong();
	dd.m_tag = tag;
	dd.m_subtag = subtag;
	dd.m_text = msg;
	return true;
}

bool Connection::handleGanttBgnCommand (DecodedCommand const & cmd)
{
	if (m_main_window->ganttState() == e_FtrDisabled)
		return true;

	gantt::DecodedData dd;
	if (!parseCommand(cmd, dd))
		return true;
	dd.m_type = gantt::e_GanttBgn;

	qDebug("+decoded Gantt type=%i tag=%s subtag=%s text=%s", dd.m_type, dd.m_tag.toStdString().c_str(), dd.m_subtag.toStdString().c_str(), dd.m_text.toStdString().c_str());
	appendGantt(dd);
	return true;
}
bool Connection::handleGanttEndCommand (DecodedCommand const & cmd)
{
	if (m_main_window->ganttState() == e_FtrDisabled)
		return true;

	gantt::DecodedData dd;
	if (!parseCommand(cmd, dd))
		return true;
	dd.m_type = gantt::e_GanttEnd;
	qDebug("+decoded Gantt type=%i tag=%s subtag=%s text=%s", dd.m_type, dd.m_tag.toStdString().c_str(), dd.m_subtag.toStdString().c_str(), dd.m_text.toStdString().c_str());
	appendGantt(dd);
	return true;
}
bool Connection::handleGanttFrameBgnCommand (DecodedCommand const & cmd)
{
	if (m_main_window->ganttState() == e_FtrDisabled)
		return true;

	gantt::DecodedData dd;
	if (!parseCommand(cmd, dd))
		return true;
	dd.m_type = gantt::e_GanttFrameBgn;
	appendGantt(dd);
	return true;

}
bool Connection::handleGanttFrameEndCommand (DecodedCommand const & cmd)
{
	if (m_main_window->ganttState() == e_FtrDisabled)
		return true;

	gantt::DecodedData dd;
	if (!parseCommand(cmd, dd))
		return true;
	dd.m_type = gantt::e_GanttFrameEnd;
	appendGantt(dd);
	return true;
}

bool Connection::loadConfigForGantt (QString const & preset_name, gantt::GanttConfig & config, QString const & tag)
{
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetGanttTag, tag);
	qDebug("gantt: load cfg file=%s", fname.toStdString().c_str());
	return loadConfig(config, fname);
}

bool Connection::loadConfigForGantts (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datagantts_t::iterator it = m_datagantts.begin(), ite = m_datagantts.end(); it != ite; ++it)
	{
		DataGantt * const tbl = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetGanttTag, tbl->m_config.m_tag);
		loadConfig(tbl->m_config, fname);
		tbl->widget().applyConfig(tbl->m_config);
		if (tbl->m_config.m_show)
			tbl->onShow();
		else
			tbl->onHide();
	}
	return true;
}

bool Connection::saveConfigForGantt (gantt::GanttConfig const & config, QString const & tag)
{
	QString const preset_name = m_curr_preset.isEmpty() ? m_main_window->getValidCurrentPresetName() : m_curr_preset;
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetGanttTag, tag);
	qDebug("gantt save cfg file=%s", fname.toStdString().c_str());
	return saveConfig(config, fname);
}

bool Connection::saveConfigForGantts (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datagantts_t::iterator it = m_datagantts.begin(), ite = m_datagantts.end(); it != ite; ++it)
	{
		DataGantt * const tbl = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetGanttTag, tbl->m_config.m_tag);
		tbl->widget().onSaveButton();
	}
	return true;
}


datagantts_t::iterator Connection::findOrCreateGantt (QString const & tag)
{
	QString const gantt_name = sessionState().getAppName() + "/" + g_presetGanttTag + "/" + tag;

	datagantts_t::iterator it = m_datagantts.find(tag);
	if (it == m_datagantts.end())
	{
		qDebug("gantt: creating gantt %s", tag.toStdString().c_str());
		// new data gantt
		gantt::GanttConfig template_config;
		template_config.m_tag = tag;

		QString const preset_name = m_main_window->matchClosestPresetName(sessionState().getAppName());
		QString fname;
		if (!preset_name.isEmpty())
		{
			fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetGanttTag, tag);
			loadConfigForGantt(preset_name, template_config, tag);
		}
		
		DataGantt * const dp = new DataGantt(this, template_config, fname);
		it = m_datagantts.insert(tag, dp);
		QModelIndex const item_idx = m_data_model->insertItemWithHint(gantt_name, template_config.m_show);

		dp->m_wd = m_main_window->m_dock_mgr.mkDockWidget(m_main_window, &dp->widget(), template_config.m_show, gantt_name);
		bool const visible = template_config.m_show;
		m_data_model->setData(item_idx, QVariant(visible ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
		if (m_main_window->ganttState() == e_FtrEnabled && visible)
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

void Connection::appendGantt (gantt::DecodedData & dd)
{
	//qDebug("appendGantt type=%i tag=%s subtag=%s text=%s", dd.m_type, dd.m_tag.toStdString().c_str(), dd.m_subtag.toStdString().c_str(), dd.m_text.toStdString().c_str());
	datagantts_t::iterator it = findOrCreateGantt(dd.m_tag);
	DataGantt & dp = **it;
	gantt::GanttView * gv = dp.widget().findOrCreateGanttView(dd.m_subtag);
	gv->appendGantt(dd);
}

/*void Connection::requestGanttSynchronization (int sync_group, unsigned long long time)
{
	for (datagantts_t::iterator it = m_datagantts.begin(), ite = m_datagantts.end(); it != ite; ++it)
	{
		DataGantt * const tbl = *it;
		if (tbl->widget().getConfig().m_sync_group == sync_group)
			tbl->widget().findNearestTimeRow(time);
	}
}

void Connection::requestGanttWheelEventSync (int sync_group, QWheelEvent * ev, QGanttView const * source)
{
	for (datagantts_t::iterator it = m_datagantts.begin(), ite = m_datagantts.end(); it != ite; ++it)
	{
		DataGantt * const tbl = *it;
		if (tbl->widget().getConfig().m_sync_group == sync_group)
			tbl->widget().requestGanttWheelEventSync(ev, source);
	}
}

void Connection::requestGanttActionSync (int sync_group, unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QGanttView const * source)
{
	for (datagantts_t::iterator it = m_datagantts.begin(), ite = m_datagantts.end(); it != ite; ++it)
	{
		DataGantt * const tbl = *it;
		if (tbl->widget().getConfig().m_sync_group == sync_group)
			tbl->widget().requestActionSync(t, cursorAction, modifiers, source);
	}
}*/

