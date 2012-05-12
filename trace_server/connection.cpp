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
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QListView>
#include <boost/tokenizer.hpp>
#include <tlv_parser/tlv_encoder.h>
#include <trace_client/trace.h>
#include "modelview.h"
#include "utils.h"


Connection::Connection (QObject * parent)
	: QThread(parent)
	, m_main_window(0)
	, m_from_file(false)
	, m_first_line(true)
	, m_last_search_row(0)
	, m_last_search_col(0)
	, m_table_view_widget(0)
	, m_tree_view_file_model(0)
	, m_tree_view_ctx_model(0)
	, m_tree_view_func_model(0)
	, m_list_view_tid_model(0)
	, m_table_view_proxy(0)
	, m_toggle_ref(0)
	, m_hide_prev(0)
	, m_exclude_fileline(0)
	, m_last_clicked()
	, m_list_view_color_regex_model(0)
	, m_buffer(e_ringbuff_size)
	, m_current_cmd()
	, m_decoded_cmds(e_ringcmd_size)
	, m_decoder()
	, m_storage(0)
	, m_datastream(0)
	, m_tcpstream(0)
{ 
	m_toggle_ref = new QAction("Toggle Ref", this);
	m_hide_prev = new QAction("Hide prev rows", this);
	m_exclude_fileline = new QAction("Exclude File:Line", this);
    m_ctx_menu.addAction(m_toggle_ref);
    m_ctx_menu.addAction(m_exclude_fileline);
}

Connection::~Connection () { qDebug("~Connection()"); }

void Connection::onDisconnected ()
{
	qDebug("onDisconnected()");
}

void Connection::onTabTraceFocus (int i)
{
	if (i != sessionState().m_tab_idx)
		return;
	m_main_window->getTreeViewFile()->setModel(m_tree_view_file_model);
	m_main_window->getTreeViewCtx()->setModel(m_tree_view_ctx_model);
	m_main_window->getListViewTID()->setModel(m_list_view_tid_model);
	m_main_window->getListViewColorRegex()->setModel(m_list_view_color_regex_model);
	//hideLinearParents();
}

void Connection::hideLinearParents ()
{
	QStandardItem * node = m_tree_view_file_model->invisibleRootItem();
	QStandardItem * last_hidden_node = 0;
	while (node)
	{
		QStandardItem * child = node->child(0);
		if (child != 0)
		{
			if (child->rowCount() == 1)
				last_hidden_node = child;
			else if (child->rowCount() > 1)
			{
				last_hidden_node = child;
				break;
			}
		}
		else
			break;

		node = child;
	}
	if (last_hidden_node)
	{
		m_main_window->getTreeViewFile()->setRootIndex(last_hidden_node->index());
	}
}

void Connection::onCloseTab ()
{
	qDebug("onCloseTab: this=%08x", this);
	if (m_tcpstream)
	{
		QObject::disconnect(m_tcpstream, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
		QObject::disconnect(m_tcpstream, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	}
	if (m_tcpstream)
		m_tcpstream->close();
	closeStorage();
	if (m_main_window->getTreeViewFile()->model() == m_tree_view_file_model)
		m_main_window->getTreeViewFile()->setModel(0);

	if (m_main_window->getTreeViewCtx()->model() == m_tree_view_ctx_model)
		m_main_window->getTreeViewCtx()->setModel(0);

	if (m_main_window->getListViewTID()->model() == m_list_view_tid_model)
		m_main_window->getListViewTID()->setModel(0);

	if (m_main_window->getListViewColorRegex()->model() == m_list_view_color_regex_model)
		m_main_window->getListViewColorRegex()->setModel(0);

	delete m_tree_view_file_model;
	m_tree_view_file_model = 0;
	delete m_tree_view_ctx_model;
	m_tree_view_ctx_model = 0;
	delete m_list_view_tid_model;
	m_list_view_tid_model = 0;
	delete m_list_view_color_regex_model;
	m_list_view_color_regex_model = 0;
	m_table_view_widget = 0;
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
	char tlv_buff[16];
#ifdef __linux__
	int const result = snprintf(tlv_buff, 16, "%u", val);
#else
	int const result = _snprintf(tlv_buff, 16, "%u", val);
#endif

	if (result > 0)
	{
		qDebug("Connection::onBufferingStateChanged to_state=%i", val);
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

void Connection::onExcludeFileLine (QModelIndex const & row_index)
{
	QString file = findString4Tag(tlv::tag_file, row_index);
	QString line = findString4Tag(tlv::tag_line, row_index);

	fileline_t filter_item(file.toStdString(), line.toStdString());
	qDebug("appending: %s:%s", file.toStdString().c_str(), line.toStdString().c_str());
	m_session_state.appendFileFilter(filter_item);
	// @TODO: update file tree view
	bool const checked = m_main_window->fltMode() == e_Exclude ? true : false;
	appendToFileFilters(file.toStdString() + "/" + line.toStdString(), checked);

	onInvalidateFilter();
}

void Connection::onTableClicked (QModelIndex const & row_index)
{
	if (m_table_view_proxy)
	{
		QModelIndex const curr = m_table_view_proxy->mapToSource(row_index);
		//qDebug("1c curr: (%i,col) -> (%i,col)", row_index.row(), curr.row());

		m_last_clicked = curr;
		QString file = findString4Tag(tlv::tag_file, curr);
		QString line = findString4Tag(tlv::tag_line, curr);

		boost::char_separator<char> sep(":/\\");
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
		std::string fstr = file.toStdString();
		tokenizer_t tok(fstr, sep);

		QStandardItem * item = m_tree_view_file_model->invisibleRootItem();
		for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
		{
			QString qfile = QString::fromStdString(*it);
			QStandardItem * child = findChildByText(item, qfile);
			if (child != 0)
			{
				item = child;
				QModelIndex idx = item->index();
				m_main_window->getTreeViewFile()->setExpanded(idx, true);
				m_main_window->getTreeViewFile()->setCurrentIndex(idx);
			}
		}

		if (item != 0)
		{
			QStandardItem * last_level = findChildByText(item, line);
			if (last_level != 0)
			{
				QModelIndex idx = last_level->index();
				m_main_window->getTreeViewFile()->setExpanded(idx, true);
				m_main_window->getTreeViewFile()->setCurrentIndex(idx);
			}
		}

		QString tid = findString4Tag(tlv::tag_tid, curr);
		QModelIndexList indexList = m_list_view_tid_model->match(m_list_view_tid_model->index(0, 0), Qt::DisplayRole, tid);
		QModelIndex const selectedIndex(indexList.first());
		m_main_window->getListViewTID()->setCurrentIndex(selectedIndex);
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
		m_session_state.toggleRefFromRow(current.row());
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

void Connection::onShowContextMenu (const QPoint& pos) // this is a slot
{
    QPoint globalPos = m_table_view_widget->mapToGlobal(pos);

    QAction * selectedItem = m_ctx_menu.exec(globalPos); // @TODO: rather async
    if (selectedItem == m_hide_prev)
    {
		onHidePrevFromRow();
    }
    if (selectedItem == m_toggle_ref)
    {
		onToggleRefFromRow();
    }
    else if (selectedItem == m_exclude_fileline)
	{
		onExcludeFileLine(m_last_clicked);
	}
    else
    { }
}

