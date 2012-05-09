#include "connection.h"
#include <QListView>
#include <QFile>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"
#include "utils.h"
#include "filterproxy.h"

/////////////// TODO: this belongs to filterproxy.cpp /////////////////////////
void FilterProxyModel::force_update ()
{
	//invalidate();
	reset();
}

bool FilterProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & /*sourceParent*/) const
{
	QString file, line;
	int const col_idx = m_session_state.findColumn4Tag(tlv::tag_file);
	if (col_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, col_idx, QModelIndex());
		file = sourceModel()->data(data_idx).toString();
	}
	int const col_idx2 = m_session_state.findColumn4Tag(tlv::tag_line);
	if (col_idx2 >= 0)
	{
		QModelIndex data_idx2 = sourceModel()->index(sourceRow, col_idx2, QModelIndex());
		line = sourceModel()->data(data_idx2).toString();
	}

	MainWindow const * mw = static_cast<Connection const *>(parent())->getMainWindow();
	bool excluded = false;
	excluded |= m_session_state.isFileLineExcluded(std::make_pair(file.toStdString(), line.toStdString()), mw->fltMode());

	QString tid;
	int const tid_idx = m_session_state.findColumn4Tag(tlv::tag_tid);
	if (tid_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, tid_idx, QModelIndex());
		tid = sourceModel()->data(data_idx).toString();
	}

	bool regex_accept = true;
	if (m_regexps.size() > 0)
	{
		regex_accept = false;
		QString msg;
		int const msg_idx = m_session_state.findColumn4Tag(tlv::tag_msg);
		if (msg_idx >= 0)
		{
			QModelIndex data_idx = sourceModel()->index(sourceRow, msg_idx, QModelIndex());
			msg = sourceModel()->data(data_idx).toString();
		}

		for (int i = 0, ie = m_regexps.size(); i < ie; ++i)
		{
			if (m_regex_user_states[i])
				regex_accept |= m_regexps[i].exactMatch(msg);
		}
	}

	excluded |= m_session_state.isTIDExcluded(tid.toStdString());

	QModelIndex data_idx = sourceModel()->index(sourceRow, 0, QModelIndex());
	excluded |= m_session_state.isBlockCollapsed(tid, data_idx.row());
	excluded |= data_idx.row() < m_session_state.excludeContentToRow();
	return !excluded && regex_accept;
}
/////////////// TODO: this belongs to filterproxy.cpp /////////////////////////


void Connection::onInvalidateFilter ()
{
	if (m_table_view_proxy)
		static_cast<FilterProxyModel *>(m_table_view_proxy)->force_update();
}

void Connection::setFilterFile (int state)
{
	if (state == Qt::Unchecked)
	{
		m_table_view_widget->setModel(m_table_view_proxy->sourceModel());
	}
	else if (state == Qt::Checked)
	{
		if (!m_table_view_proxy)
		{
			m_table_view_proxy = new FilterProxyModel(this, m_main_window->getRegexps(), m_main_window->getRegexUserStates(), m_session_state);

			m_table_view_proxy->setSourceModel(m_table_view_widget->model());
			m_table_view_widget->setModel(m_table_view_proxy);
			m_table_view_proxy->setDynamicSortFilter(true);
		}
		else
		{
			m_table_view_widget->setModel(m_table_view_proxy);
		}
	}
	m_main_window->getTreeViewFile()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getTreeViewCtx()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getListViewTID()->setEnabled(m_main_window->filterEnabled());
}

void Connection::clearFilters (QStandardItem * node)
{
	if (node)
	{
		if (node->checkState() == Qt::Checked)
		{
			node->setCheckState(Qt::Unchecked);
		}
		for (int i = 0, ie = node->rowCount(); i < ie; ++i)
			clearFilters(node->child(i));
	}
}

void Connection::clearFilters ()
{
	QStandardItem * node = m_tree_view_file_model->invisibleRootItem();
	clearFilters(node);
	sessionState().m_file_filters.clear();
	sessionState().m_tid_filters.clear();
	//sessionState().m_regex_filters.clear();
	//sessionState().m_color_regexps.clear();
}

void Connection::appendToFileFilters (std::string const & item, bool checked)
{
	boost::char_separator<char> sep(":/\\");
	appendToFileFilters(sep, item, checked);
}

void Connection::appendToFileFilters (boost::char_separator<char> const & sep, std::string const & item, bool checked)
{
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
	tokenizer_t tok(item, sep);

	QStandardItem * node = m_tree_view_file_model->invisibleRootItem();
	QStandardItem * last_hidden_node = 0;
	bool append = false;
	bool stop = false;
	for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
	{
		QString qItem = QString::fromStdString(*it);
		QStandardItem * child = findChildByText(node, qItem);
		if (child != 0)
		{
			node = child;
			if (!stop)
			{
				if (child->rowCount() == 1)
				{
					last_hidden_node = node;
				}
				else if (child->rowCount() > 1)
				{
					stop = true;
					last_hidden_node = node;
				}
			}
		}
		else
		{
			stop = true;
			append = true;
			QList<QStandardItem *> row_items = addRow(qItem, checked);
			node->appendRow(row_items);
			node = row_items.at(0);
		}
	}
	if (last_hidden_node)
	{
		m_main_window->getTreeViewFile()->setRootIndex(last_hidden_node->index());
	}
	if (!append && checked)
	{
		node->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	}
}

void Connection::appendToFileFilters (boost::char_separator<char> const & sep, std::string const & file, std::string const & line)
{
	appendToFileFilters(sep, file + "/" + line);
}

void Connection::appendToTIDFilters (std::string const & item)
{
	QString qItem = QString::fromStdString(item);
	QStandardItem * root = m_list_view_tid_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(qItem, false);
		root->appendRow(row_items);
	}
}

void Connection::appendToCtxFilters (std::string const & item, bool checked)
{
	QString qItem = QString::fromStdString(item);
	QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getTreeViewCtx()->model());
	QStandardItem * root = model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(qItem, false);
		root->appendRow(row_items);
	}
}


bool Connection::appendToFilters (DecodedCommand const & cmd)
{
	std::string line;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_line)
		{
			line = cmd.tvs[i].m_val;
			break;
		}

		if (cmd.tvs[i].m_tag == tlv::tag_tid)
		{
			int idx = sessionState().m_tls.findThreadId(cmd.tvs[i].m_val);
			if (cmd.hdr.cmd == tlv::cmd_scope_entry)
				sessionState().m_tls.incrIndent(idx);
			if (cmd.hdr.cmd == tlv::cmd_scope_exit)
				sessionState().m_tls.decrIndent(idx);
			appendToTIDFilters(cmd.tvs[i].m_val);
		}

		if (cmd.tvs[i].m_tag == tlv::tag_ctx)
		{
			appendToCtxFilters(cmd.tvs[i].m_val, false);
		}
	}

	boost::char_separator<char> sep(":/\\");

	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_file)
		{
			std::string file(cmd.tvs[i].m_val);
			appendToFileFilters(sep, file, line);
		}
	}
	return true;
}

void Connection::appendToColorRegexFilters (std::string const & val)
{
	m_session_state.appendToColorRegexFilters(val);
}

void Connection::removeFromColorRegexFilters (std::string const & val)
{
	m_session_state.removeFromColorRegexFilters(val);
}

void Connection::recompileColorRegexps ()
{
	m_color_regexps.clear();
	m_color_regex_user_states.clear();

	for (int i = 0, ie = m_filter_color_regexs.size(); i < ie; ++i)
	{
		QStandardItem * root = m_list_view_color_regex_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, m_filter_color_regexs.at(i));
		QRegExp regex(QRegExp(m_filter_color_regexs.at(i)));
		if (regex.isValid())
		{
			m_color_regexps.append(regex);
			m_color_regex_user_states.push_back(false);

			bool const checked = (child->checkState() == Qt::Checked);
			if (child && checked)
			{
				child->setData(QBrush(Qt::green), Qt::BackgroundRole);
				child->setToolTip(tr("ok"));
				m_color_regex_user_states.back() = true;
			}
			else if (child && !checked)
			{
				child->setData(QBrush(Qt::yellow), Qt::BackgroundRole);
				child->setToolTip(tr("not checked"));
			}
		}
		else
		{
			if (child)
			{
				child->setData(QBrush(Qt::red), Qt::BackgroundRole);
				child->setToolTip(regex.errorString());
			}
		}
	}

	onInvalidateFilter();
}



