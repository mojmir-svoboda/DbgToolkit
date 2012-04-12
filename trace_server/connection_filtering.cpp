#include "connection.h"
#include <QListView>
#include <QFile>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"
#include "utils.h"

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
	//@TODO: clear TID Filter
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



