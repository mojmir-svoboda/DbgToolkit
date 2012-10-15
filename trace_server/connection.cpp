#include "connection.h"
#include <QtNetwork>
#include <QCheckBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QFile>
#include <QDataStream>
#include <QMenu>
#include <QTime>
#include <QTimer>
#include <QStandardItemModel>
#include <QListView>
#include <boost/tokenizer.hpp>
#include <tlv_parser/tlv_encoder.h>
#include <trace_client/trace.h>
#include "modelview.h"
#include "utils.h"
#include "utils_qstandarditem.h"
#include "types.h"
#include "statswindow.h"
#include "delegates.h"

Connection::Connection (QObject * parent)
	: QThread(parent)
	, m_main_window(0)
	, m_from_file(false)
	, m_column_setup_done(false)
	, m_marked_for_close(false)
	, m_last_search_row(0)
	, m_last_search_col(0)
	, m_table_view_widget(0)
	, m_file_model(0)
	, m_ctx_model(0)
	, m_func_model(0)
	, m_tid_model(0)
	, m_color_regex_model(0)
	, m_regex_model(0)
	, m_lvl_model(0)
	, m_lvl_delegate(0)
	, m_ctx_delegate(0)
	, m_table_view_proxy(0)
	, m_table_view_src(0)
	, m_toggle_ref(0)
	, m_hide_prev(0)
	, m_exclude_fileline(0)
	, m_copy_to_clipboard(0)
	, m_last_clicked()
	, m_buffer(e_ringbuff_size)
	, m_current_cmd()
	, m_decoded_cmds(e_ringcmd_size)
	, m_decoder()
	, m_storage(0)
	, m_datastream(0)
	, m_tcpstream(0)
	, m_statswindow(0)
	, m_data_model(0)
{
	qDebug("Connection::Connection() this=0x%08x", this);
	m_copy_to_clipboard = new QAction("Copy", this);
	m_hide_prev = new QAction("Hide prev rows", this);
	m_toggle_ref = new QAction("Set as reference time", this);
	m_exclude_fileline = new QAction("Exclude File:Line", this);
    m_ctx_menu.addAction(m_toggle_ref);
    m_ctx_menu.addAction(m_exclude_fileline);
    m_ctx_menu.addAction(m_copy_to_clipboard);
	m_data_model = new TreeModel(this, &m_session_state.m_data_filters);
	m_lvl_delegate = new LevelDelegate(m_session_state, this);
	m_ctx_delegate = new CtxDelegate(m_session_state, this);
}

namespace {

	template <class ContainerT>
	void destroyDockedWidgets (ContainerT & c, QMainWindow & mainwin)
	{
		for (typename ContainerT::iterator it = c.begin(), ite = c.end(); it != ite; ++it)
		{
			typename ContainerT::iterator::value_type ptr = *it;
			if (ptr->m_wd)
			{
				mainwin.removeDockWidget(ptr->m_wd);
				ptr->getWidget().setParent(0);
				ptr->m_wd->setWidget(0);
				delete ptr->m_wd;
			}
			delete ptr;
		}
		c.clear();
	}
}

Connection::~Connection ()
{
	for (datatables_t::iterator it = m_datatables.begin(), ite = m_datatables.end(); it != ite; ++it)
		QObject::disconnect((*it)->m_table.horizontalHeader(), SIGNAL(sectionResized(int, int, int)), &(*it)->m_table, SLOT(onSectionResized(int, int, int)));
	qDebug("Connection::~Connection() this=0x%08x", this);
	if (m_statswindow)
	{
		delete m_statswindow;
		m_statswindow = 0;
	}

	destroyDockedWidgets(m_dataplots, *m_main_window);
	destroyDockedWidgets(m_datatables, *m_main_window);

	if (m_tcpstream)
	{
		QObject::disconnect(m_tcpstream, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
		QObject::disconnect(m_tcpstream, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	}
	if (m_tcpstream)
		m_tcpstream->close();
	closeStorage();

	destroyModelFile();

	if (m_main_window->getWidgetLvl()->itemDelegate() == m_lvl_delegate)
		m_main_window->getWidgetLvl()->setItemDelegate(0);
	if (m_main_window->getWidgetLvl()->model() == m_lvl_model)
		m_main_window->getWidgetLvl()->setModel(0);
	delete m_lvl_model;
	m_lvl_model = 0;
	delete m_lvl_delegate;
	m_lvl_delegate = 0;

	if (m_main_window->getWidgetCtx()->itemDelegate() == m_ctx_delegate)
		m_main_window->getWidgetCtx()->setItemDelegate(0);
	if (m_main_window->getWidgetCtx()->model() == m_ctx_model)
		m_main_window->getWidgetCtx()->setModel(0);
	delete m_ctx_model;
	m_ctx_model = 0;
	delete m_ctx_delegate;
	m_ctx_delegate = 0;

	if (m_main_window->getWidgetTID()->model() == m_tid_model)
		m_main_window->getWidgetTID()->setModel(0);
	delete m_tid_model;
	m_tid_model = 0;

	if (m_main_window->getWidgetColorRegex()->model() == m_color_regex_model)
		m_main_window->getWidgetColorRegex()->setModel(0);
	delete m_color_regex_model;
	m_color_regex_model = 0;


	if (m_table_view_proxy)
	{
		m_table_view_proxy->setSourceModel(0);
		delete m_table_view_proxy;
		m_table_view_proxy = 0;
	}

	m_table_view_widget->setModel(0);
	delete m_table_view_src;
	m_table_view_src = 0;

	m_table_view_widget = 0;
}

void Connection::onDisconnected ()
{
	qDebug("onDisconnected()");
	if (m_statswindow)
		m_statswindow->stopUpdate();

	if (m_main_window->dumpModeEnabled())
	{
		QString fname = tr("%1_%2.csv").arg(sessionState().m_name).arg(sessionState().m_pid);
		exportStorageToCSV(fname);

		Server * server = static_cast<Server *>(parent());
		m_marked_for_close = true;
		QTimer::singleShot(0, server, SLOT(onCloseMarkedTabs()));
	}

	for (dataplots_t::iterator it = m_dataplots.begin(), ite = m_dataplots.end(); it != ite; ++it)
	{
		DataPlot * dp = (*it);
		dp->m_plot.stopUpdate();
	}
}

void Connection::onTabTraceFocus ()
{
	setupModelFile();
	setupModelLvl();
	m_main_window->getWidgetCtx()->setModel(m_ctx_model);
	m_main_window->getWidgetTID()->setModel(m_tid_model);
	m_main_window->getWidgetColorRegex()->setModel(m_color_regex_model);
	m_main_window->getWidgetRegex()->setModel(m_regex_model);
}

void Connection::onLevelValueChanged (int val)
{
	char tlv_buff[16];
#ifdef __linux__
	int const result = snprintf(tlv_buff, 16, "%u", val);
#else
	int const result = _snprintf(tlv_buff, 16, "%u", val);
#endif

	if (result > 0)
	{
		char buff[256];
		using namespace tlv;
		Encoder e(cmd_set_level, buff, 256);
		e.Encode(TLV(tag_lvl, tlv_buff));
		if (m_tcpstream && e.Commit())
			m_tcpstream->write(e.buffer, e.total_len); /// @TODO: async write
	}
}

void Connection::onBufferingStateChanged (int val)
{
	bool const buffering_enabled = (val == Qt::Checked) ? true : false;

	char tlv_buff[16];
#ifdef __linux__
	int const result = snprintf(tlv_buff, 16, "%u", buffering_enabled);
#else
	int const result = _snprintf(tlv_buff, 16, "%u", buffering_enabled);
#endif

	if (result > 0)
	{
		qDebug("Connection::onBufferingStateChanged to_state=%i", buffering_enabled);
		char buff[256];
		using namespace tlv;
		Encoder e(cmd_set_buffering, buff, 256);
		e.Encode(TLV(tag_bool, tlv_buff));
		if (m_tcpstream && e.Commit())
			m_tcpstream->write(e.buffer, e.total_len); /// @TODO: async write
	}
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

void Connection::onTableClicked (QModelIndex const & row_index)
{
	if (m_table_view_proxy)
	{
		QModelIndex const curr = m_table_view_proxy->mapToSource(row_index);

		//qDebug("1c curr: (%i,col) -> (%i,col)", row_index.row(), curr.row());

		m_last_clicked = curr;

		{
			QString const file = findString4Tag(tlv::tag_file, curr);
			QString const line = findString4Tag(tlv::tag_line, curr);
			QString const combined = file + "/" + line;
			m_file_model->selectItem(m_main_window->getWidgetFile(), combined.toStdString());
		}

		{
			QString tid = findString4Tag(tlv::tag_tid, curr);
			QModelIndexList indexList = m_tid_model->match(m_tid_model->index(0, 0), Qt::DisplayRole, tid);
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetTID()->setCurrentIndex(selectedIndex);
		}
		{
			QString lvl = findString4Tag(tlv::tag_lvl, curr);
			QModelIndexList indexList = m_lvl_model->match(m_lvl_model->index(0, 0), Qt::DisplayRole, lvl);
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetLvl()->setCurrentIndex(selectedIndex);
		}
		{
			QString ctx = findString4Tag(tlv::tag_ctx, curr);
			QModelIndexList indexList = m_ctx_model->match(m_ctx_model->index(0, 0), Qt::DisplayRole, ctx);
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetCtx()->setCurrentIndex(selectedIndex);
		}

		m_last_search_row = curr.row(); // set search from this line
	}
	else
		m_last_search_row = row_index.row(); // set search from this line
}

void Connection::onTableDoubleClicked (QModelIndex const & row_index)
{
	if (m_table_view_proxy)
	{
		QModelIndex const curr = m_table_view_proxy->mapToSource(row_index);
		ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
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
					if (model->layers()[row_bgn] >= layer)
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
			while (row_end < model->layers().size())
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

	if (sessionState().m_app_idx == -1)
		sessionState().m_app_idx = m_main_window->m_config.m_columns_setup.size() - 1;
	
	columns_setup_t const & new_cs = m_main_window->getColumnSetup(sessionState().m_app_idx);
	if (0 == sessionState().m_columns_setup_current)
	{
	}
	else
	{
		columns_setup_t const & old_cs = *sessionState().m_columns_setup_current;
	}

	//m_table_view_widget->horizontalHeader()->moveSection(from, to);
	//m_table_view_widget->horizontalHeader()->swapSection(from, to);
	//m_table_view_widget->horizontalHeader()->hideSection(from, to);
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

		ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
		QString const & strtime = findString4Tag(tlv::tag_time, current);
		m_session_state.setTimeRefValue(strtime.toULongLong());
		onInvalidateFilter();
	}
}

void Connection::onClearCurrentView ()
{
	//QModelIndex const curr = m_table_view_proxy->mapToSource(row_index);
	ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
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
    else if (selectedItem == m_copy_to_clipboard)
	{
		onCopyToClipboard();
	}
    else
    { }
}

void Connection::onSaveAll ()
{
	// @TODO: v hhdr bude 0 !
	for (dataplots_t::iterator it = m_dataplots.begin(), ite = m_dataplots.end(); it != ite; ++it)
	{
		DataPlot * dp = (*it);
		dp->getWidget().onSaveButton();
	}

	for (datatables_t::iterator it = m_datatables.begin(), ite = m_datatables.end(); it != ite; ++it)
	{
		DataTable * dt = (*it);
		dt->getWidget().onSaveButton();
	}	
}

