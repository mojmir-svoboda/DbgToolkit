#include "connection.h"
#include <QClipboard>
#include "logtablemodel.h"
#include "tableview.h"
#include "constants.h"
#include "utils.h"

DataLog::DataLog (Connection * parent, logs::LogConfig & config, QString const & fname)
	: m_parent(parent)
	, m_wd(0)
	, m_config(config)
	, m_widget(0)
	, m_fname(fname)
{
	qDebug("%s this=0x%08x name=%s", __FUNCTION__, this, fname.toStdString().c_str());
	m_widget = new logs::BaseLog(parent, 0, m_config, fname);
}
DataLog::~DataLog ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	delete m_widget;
	m_widget = 0;
}
void DataLog::onShow ()
{
	m_widget->onShow();
	m_wd->show();
	m_parent->getMainWindow()->restoreDockWidget(m_wd);
	//QTimer::singleShot(0, m_parent, SLOT(onShowLogs()));
}
void DataLog::onHide ()
{
	m_widget->onHide();
	QTimer::singleShot(0, m_wd, SLOT(hide()));
}

bool Connection::handleLogCommand (DecodedCommand const & cmd)
{
	appendToFilters(cmd);

	if (cmd.hdr.cmd == tlv::cmd_scope_entry || (cmd.hdr.cmd == tlv::cmd_scope_exit))
	{
		if (m_main_window->scopesEnabled())
		{
			LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
			model->appendCommand(m_table_view_proxy, cmd);
		}
	}
	else if (cmd.hdr.cmd == tlv::cmd_log)
	{
		LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
		model->appendCommand(m_table_view_proxy, cmd);
	}

	m_main_window->getWidgetFile()->hideLinearParents();
	return true;
}


bool Connection::handleLogClearCommand (DecodedCommand const & cmd)
{
	return true;
}


void Connection::onShowLogs ()
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_datalogs.begin(), ite = m_datalogs.end(); it != ite; ++it)
	{
		(*it)->onShow();
		m_main_window->restoreDockWidget((*it)->m_wd);
	}
}

void Connection::onHideLogs ()
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_datalogs.begin(), ite = m_datalogs.end(); it != ite; ++it)
	{
		(*it)->onHide();
	}
}

void Connection::onShowLogContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_datalogs.begin(), ite = m_datalogs.end(); it != ite; ++it)
	{
		(*it)->widget().onHideContextMenu();
	}
}

bool Connection::loadConfigForLog (QString const & preset_name, logs::LogConfig & config, QString const & tag)
{
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetGanttTag, tag);
	qDebug("logs: load cfg file=%s", fname.toStdString().c_str());
	return loadConfig(config, fname);
}


bool Connection::loadConfigForLogs (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datalogs_t::iterator it = m_datalogs.begin(), ite = m_datalogs.end(); it != ite; ++it)
	{
		DataLog * const tbl = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetLogTag, tbl->m_config.m_tag);
		loadConfig(tbl->m_config, fname);
		tbl->widget().applyConfig(tbl->m_config);
		if (tbl->m_config.m_show)
			tbl->onShow();
		else
			tbl->onHide();
	}
	return true;
}

bool Connection::saveConfigForLog (logs::LogConfig const & config, QString const & tag)
{
	QString const preset_name = m_curr_preset.isEmpty() ? m_main_window->getValidCurrentPresetName() : m_curr_preset;
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetLogTag, tag);
	qDebug("log save cfg file=%s", fname.toStdString().c_str());
	return saveConfig(config, fname);
}

bool Connection::saveConfigForLogs (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datalogs_t::iterator it = m_datalogs.begin(), ite = m_datalogs.end(); it != ite; ++it)
	{
		DataLog * const tbl = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetLogTag, tbl->m_config.m_tag);
		tbl->widget().onSaveButton();
	}
	return true;
}


datalogs_t::iterator Connection::findOrCreateLog (QString const & tag)
{
	QString const log_name = sessionState().getAppName() + "/" + g_presetLogTag + "/" + tag;

	datalogs_t::iterator it = m_datalogs.find(tag);
	if (it == m_datalogs.end())
	{
		qDebug("log: creating log %s", tag.toStdString().c_str());
		// new data log
		logs::LogConfig template_config;
		template_config.m_tag = tag;

		QString const preset_name = m_main_window->matchClosestPresetName(sessionState().getAppName());
		QString fname;
		if (!preset_name.isEmpty())
		{
			fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetLogTag, tag);
			loadConfigForLog(preset_name, template_config, tag);
		}
		
		DataLog * const dp = new DataLog(this, template_config, fname);
		it = m_datalogs.insert(tag, dp);
		QModelIndex const item_idx = m_data_model->insertItemWithHint(log_name, template_config.m_show);

		dp->m_wd = m_main_window->m_dock_mgr.mkDockWidget(m_main_window, &dp->widget(), template_config.m_show, log_name);
		bool const visible = template_config.m_show;
		m_data_model->setData(item_idx, QVariant(visible ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
		//if (m_main_window->logState() == e_FtrEnabled && visible)
		{
			m_main_window->loadLayout(preset_name);
			dp->onShow();
		}
		/*else
		{
			dp->onHide();
		}*/
	}
	return it;
}

/*void Connection::appendLog (log::DecodedData & dd)
{
	//qDebug("appendLog type=%i tag=%s subtag=%s text=%s", dd.m_type, dd.m_tag.toStdString().c_str(), dd.m_subtag.toStdString().c_str(), dd.m_text.toStdString().c_str());
	datalogs_t::iterator it = findOrCreateLog(dd.m_tag);
	DataLog & dp = **it;
	log::LogView * gv = dp.widget().findOrCreateLogView(dd.m_subtag);
	gv->appendLog(dd);
}*/

void Connection::onTabTraceFocus ()
{
	setupModelFile();
	setupModelLvl();
	m_main_window->getWidgetCtx()->setModel(m_ctx_model);
	m_main_window->getWidgetTID()->setModel(m_tid_model);
	m_main_window->getWidgetColorRegex()->setModel(m_color_regex_model);
	m_main_window->getWidgetRegex()->setModel(m_regex_model);
	m_main_window->getWidgetString()->setModel(m_string_model);
	if (!m_curr_preset.isEmpty())
		m_main_window->setPresetNameIntoComboBox(m_curr_preset);
	m_main_window->setLastSearchIntoCombobox(m_last_search);
}

QString Connection::onCopyToClipboard ()
{
	QAbstractItemModel * model = m_table_view_widget->model();
	QItemSelectionModel * selection = m_table_view_widget->selectionModel();
	QModelIndexList indexes = selection->selectedIndexes();

	if (indexes.size() < 1)
		return QString();

	std::sort(indexes.begin(), indexes.end());

	QString selected_text;
	selected_text.reserve(4096);
	for (int i = 0; i < indexes.size(); ++i)
	{
		QModelIndex const current = indexes.at(i);
		selected_text.append(model->data(current).toString());
		
		if (i + 1 < indexes.size() && current.row() != indexes.at(i + 1).row())
			selected_text.append('\n');	// switching rows
		else
			selected_text.append('\t');
	}
	return selected_text;
}

bool Connection::isModelProxy () const
{
	if (0 == m_table_view_widget->model())
		return false;
	return m_table_view_widget->model() == m_table_view_proxy;
}

void Connection::findTableIndexInFilters (QModelIndex const & src_idx, bool scroll_to_item, bool expand)
{
	{
		QString const file = findString4Tag(tlv::tag_file, src_idx);
		QString const line = findString4Tag(tlv::tag_line, src_idx);
		QString const combined = file + "/" + line;
		qDebug("findTableIndexInFilters %s in tree", combined.toStdString().c_str());
		m_file_model->selectItem(m_main_window->getWidgetFile(), combined, scroll_to_item);
		if (expand)
			m_file_model->expandItem(m_main_window->getWidgetFile(), combined);
	}
	{
		QString tid = findString4Tag(tlv::tag_tid, src_idx);
		QModelIndexList indexList = m_tid_model->match(m_tid_model->index(0, 0), Qt::DisplayRole, tid);
		if (!indexList.empty())
		{
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetTID()->setCurrentIndex(selectedIndex);
		}
	}
	{
		QString lvl = findString4Tag(tlv::tag_lvl, src_idx);
		QModelIndexList indexList = m_lvl_model->match(m_lvl_model->index(0, 0), Qt::DisplayRole, lvl);
		if (!indexList.empty())
		{
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetLvl()->setCurrentIndex(selectedIndex);
		}
	}
	{
		QString ctx = findString4Tag(tlv::tag_ctx, src_idx);
		QModelIndexList indexList = m_ctx_model->match(m_ctx_model->index(0, 0), Qt::DisplayRole, ctx);
		if (!indexList.empty())
		{
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetCtx()->setCurrentIndex(selectedIndex);
		}
	}
}

void Connection::onTableClicked (QModelIndex const & row_index)
{
	m_last_clicked = row_index;
	if (isModelProxy())
		m_last_clicked = m_table_view_proxy->mapToSource(row_index);

	m_last_search_row = m_last_clicked.row(); // set search from this line
	m_last_search_col = m_last_clicked.column();

	bool const scroll_to_item = false;
	bool const expand = false;
	findTableIndexInFilters(m_last_clicked, scroll_to_item, expand);
}

void Connection::onTableDoubleClicked (QModelIndex const & row_index)
{
	if (m_table_view_proxy)
	{
		QModelIndex const curr = m_table_view_proxy->mapToSource(row_index);
		LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
		qDebug("2c curr: (%i,col) -> (%i,col)", row_index.row(), curr.row());

		int row_bgn = curr.row();
		int row_end = curr.row();
		int layer = model->layers()[row_bgn];

		if (model->rowTypes()[row_bgn] == tlv::cmd_scope_exit)
		{
			layer += 1;
			// test na range
			--row_bgn;
		}

		QString tid = findString4Tag(tlv::tag_tid, curr);
		QString file = findString4Tag(tlv::tag_file, curr);
		QString line = findString4Tag(tlv::tag_line, curr);
		int from = row_bgn;

		if (model->rowTypes()[from] != tlv::cmd_scope_entry)
		{
			while (row_bgn > 0)
			{
				QModelIndex const curr_idx = model->index(row_bgn, row_index.column(), QModelIndex());
				QString curr_tid = findString4Tag(tlv::tag_tid, curr_idx);
				if (curr_tid == tid)
				{
					if (static_cast<int>(model->layers()[row_bgn]) >= layer)
					{
						from = row_bgn;
					}
					else
					{	
						break;
					}
				}
				--row_bgn;
			}
		}

		int to = row_end;
		if (model->rowTypes()[to] != tlv::cmd_scope_exit)
		{
			while (row_end < static_cast<int>(model->layers().size()))
			{
				QModelIndex const curr_idx = model->index(row_end, row_index.column(), QModelIndex());
				QString next_tid = findString4Tag(tlv::tag_tid, curr_idx);
				if (next_tid == tid)
				{
					if (model->layers()[row_end] >= layer)
						to = row_end;
					else if ((model->layers()[row_end] == layer - 1) && (model->rowTypes()[row_end] == tlv::cmd_scope_exit))
					{
						to = row_end;
						break;
					}
					else
						break;
				}
				++row_end;
			}
		}

		qDebug("row=%u / curr=%u layer=%u, from,to=(%u, %u)", row_index.row(), curr.row(), layer, from, to);
		if (m_session_state.findCollapsedBlock(tid, from, to))
			m_session_state.eraseCollapsedBlock(tid, from, to);
		else
			m_session_state.appendCollapsedBlock(tid, from, to, file, line);
		onInvalidateFilter();
	}
}

void Connection::onApplyColumnSetup ()
{
	qDebug("%s", __FUNCTION__);
	for (int i = 0; i < m_table_view_widget->horizontalHeader()->count(); ++i)
	{
		//qDebug("column: %s", m_table_view_widget->horizontalHeader()->text());
	}

	QMap<int, int> order;

	if (sessionState().m_app_idx == -1)
		sessionState().m_app_idx = m_main_window->m_config.m_columns_setup.size() - 1;
	
	columns_setup_t const & new_cs = m_main_window->getColumnSetup(sessionState().m_app_idx);

	for (int i = 0, ie = new_cs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = tlv::tag_for_name(new_cs.at(i).toStdString().c_str());
		if (tag != tlv::tag_invalid)
		{
			order[tag] = i;
		}
	}

	if (0 == sessionState().m_columns_setup_current)
	{
	}
	else
	{
		columns_setup_t const & old_cs = *sessionState().m_columns_setup_current;
	}

	static_cast<TableView *>(m_table_view_widget)->setColumnOrder(order, m_session_state);
}

void Connection::onExcludeFileLine ()
{
	QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		onExcludeFileLine(current);
		onInvalidateFilter();
	}
}

void Connection::onToggleRefFromRow ()
{
	QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		qDebug("Toggle Ref from row=%i", current.row());
		m_session_state.setTimeRefFromRow(current.row());

		//LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
		QString const & strtime = findString4Tag(tlv::tag_time, current);
		m_session_state.setTimeRefValue(strtime.toULongLong());
		onInvalidateFilter();
	}
}

void Connection::onColorTagRow (int)
{
	QModelIndex current = m_table_view_widget->currentIndex();
	if (isModelProxy())
	{
		current = m_table_view_proxy->mapToSource(current);
	}

	int const row = current.row(); // set search from this line
	if (current.isValid())
	{
		qDebug("Color tag on row=%i", current.row());
		m_session_state.addColorTagRow(current.row());
		onInvalidateFilter();
	}
}

void Connection::onClearCurrentView ()
{
	//QModelIndex const curr = m_table_view_proxy->mapToSource(row_index);
	LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	m_session_state.excludeContentToRow(model->rowCount());
	onInvalidateFilter();
}

void Connection::onHidePrevFromRow ()
{
	QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		qDebug("Hide prev from row=%i", current.row());
		m_session_state.excludeContentToRow(current.row());
		onInvalidateFilter();
	}
}

void Connection::onUnhidePrevFromRow ()
{
	m_session_state.excludeContentToRow(0);
	onInvalidateFilter();
}

void Connection::onShowContextMenu (QPoint const & pos)
{
    QPoint globalPos = m_table_view_widget->mapToGlobal(pos);
    QAction * selectedItem = m_ctx_menu.exec(globalPos); // @TODO: rather async

	//poas = ui->tableView->viewport()->mapFromGlobal(e->globalPos());
	QModelIndex const idx = m_table_view_widget->indexAt(pos);
	qDebug("left click at r=%2i,c=%2i", idx.row(), idx.column());

	onTableClicked(idx);

    if (selectedItem == m_hide_prev)
    {
		onHidePrevFromRow();
    }
    else if (selectedItem == m_toggle_ref)
    {
		onToggleRefFromRow();
    }
    else if (selectedItem == m_exclude_fileline)
	{
		onExcludeFileLine(m_last_clicked);
	}
    else if (selectedItem == m_find_fileline)
	{
		onFindFileLine(m_last_clicked);
	}
    else if (selectedItem == m_copy_to_clipboard)
	{
		QString const & selection = onCopyToClipboard();
		qApp->clipboard()->setText(selection);
	}
    else if (selectedItem == m_color_tag_row)
	{
		onColorTagRow(m_last_clicked.row());
	}
    else
    { }
}

