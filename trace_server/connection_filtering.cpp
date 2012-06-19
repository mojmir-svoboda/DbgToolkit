#include "connection.h"
#include <QListView>
#include <QFile>
#include <QRegExp>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"
#include "utils.h"
#include "filterproxy.h"

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
			m_table_view_proxy = new FilterProxyModel(this, m_session_state);

			m_table_view_proxy->setSourceModel(m_table_view_widget->model());
			m_table_view_widget->setModel(m_table_view_proxy);
		}
		else
		{
			m_table_view_widget->setModel(m_table_view_proxy);
		}
	}
	m_main_window->getWidgetFile()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getWidgetCtx()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getWidgetTID()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getWidgetColorRegex()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getWidgetRegex()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getWidgetLvl()->setEnabled(m_main_window->filterEnabled());
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
	//@TODO: call all functions below
	//sessionState().clearFilters();
}

void Connection::onClearCurrentFileFilter ()
{
	QStandardItem * node = m_file_model->invisibleRootItem();
	E_FilterMode const fmode = m_main_window->fltMode();
	setCheckStateChilds(node->child(0,0), fmode == e_Include ? Qt::Checked : Qt::Unchecked);
	sessionState().onClearFileFilter();
	onInvalidateFilter();
}
void Connection::onClearCurrentCtxFilter ()
{
	sessionState().onClearCtxFilter();
	// @TODO: checkboxes
	onInvalidateFilter();
}
void Connection::onClearCurrentTIDFilter ()
{
	sessionState().onClearTIDFilter();
	onInvalidateFilter();
}
void Connection::onClearCurrentColorizedRegexFilter ()
{
	sessionState().onClearColorizedRegexFilter();
	onInvalidateFilter();
}
void Connection::onClearCurrentScopeFilter ()
{
	sessionState().onClearScopeFilter();
	onInvalidateFilter();
}

void Connection::appendToFileTree (boost::char_separator<char> const & sep, std::string const & item)
{
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
	tokenizer_t tok(item, sep);

	E_FilterMode const fmode = m_main_window->fltMode();
	QStandardItem * node = m_file_model->invisibleRootItem();
	QStandardItem * last_hidden_node = 0;
	bool append = false;
	bool stop = false;
	std::string path;
	for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
	{
		QString qItem = QString::fromStdString(*it);
		QStandardItem * child = findChildByText(node, qItem);
		path += "/";
		path += *it;
		if (child != 0)
		{
			node = child;
			E_NodeStates ff_state = e_Unchecked;
			bool const known = sessionState().m_file_filters.is_present(path, ff_state);
			if (known)
				node->setCheckState(static_cast<Qt::CheckState>(ff_state));

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
			E_NodeStates new_state = e_Checked;

			E_NodeStates ff_state = e_Unchecked;
			bool const known = sessionState().m_file_filters.is_present(path, ff_state);
			if (known)
			{
				node->setCheckState(static_cast<Qt::CheckState>(ff_state));
				new_state = ff_state;
			}
			else
			{
				new_state = e_Checked;
				qDebug("unknown: %s, state=%u", path.c_str(), new_state);
				sessionState().m_file_filters.set_to_state(item, static_cast<E_NodeStates>(new_state));
			}

			//qDebug("new node: %s, state=%u", path.c_str(), new_state);
			QList<QStandardItem *> row_items = addRowTriState(qItem, new_state);
			node->appendRow(row_items);
			node = row_items.at(0);
		}
	}
	if (last_hidden_node)
	{
		if (last_hidden_node->parent())
			m_main_window->getWidgetFile()->setRootIndex(last_hidden_node->parent()->index());
		else
			m_main_window->getWidgetFile()->setRootIndex(last_hidden_node->index());
	}
}

void Connection::appendToTIDFilters (std::string const & item)
{
	QString qItem = QString::fromStdString(item);
	QStandardItem * root = m_tid_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	E_FilterMode const fmode = m_main_window->fltMode();
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(qItem, fmode == e_Include ? Qt::Checked : Qt::Unchecked);
		root->appendRow(row_items);
	}
}

void Connection::appendToLvlFilters (std::string const & item, bool checked)
{
	QString qItem = QString::fromStdString(item);
	QStandardItem * root = m_lvl_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	E_FilterMode const fmode = m_main_window->fltMode();
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(qItem, fmode == e_Include ? Qt::Checked : Qt::Unchecked);
		root->appendRow(row_items);
	}
}

void Connection::appendToCtxFilters (std::string const & item, bool checked)
{
	QString qItem = QString::fromStdString(item);
	QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetCtx()->model());
	QStandardItem * root = model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	E_FilterMode const fmode = m_main_window->fltMode();
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(qItem, fmode == e_Include ? Qt::Checked : Qt::Unchecked);
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
		if (cmd.tvs[i].m_tag == tlv::tag_lvl)
		{
			appendToLvlFilters(cmd.tvs[i].m_val, false);
		}
	}

	boost::char_separator<char> sep(":/\\");

	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_file)
		{
			std::string file(cmd.tvs[i].m_val);
			appendToFileTree(sep, file + "/" + line);

			//_FilterMode fmode = m_main_window->fltMode();
			//E_NodeStates state = e_Checked;
			//bool const present = sessionState().isFileLinePresent(fileline_t(file, line), excluded);
			//if (!present)
			{
				//@TODO: stav checkboxu 
				// if inclusive && parent == partial --> unchecked
				// if inclusive && parent == checked --> checked
				// if inclusive && parent == unchecked --> unchecked
			}
			//bool const default_checked = fmode == e_Exclude ? false : true;
			//bool const checked = present ? (fmode == e_Exclude ? excluded : !excluded) : default_checked;
			//appendToFileFilters(sep, file, line, checked);
		}
	}
	return true;
}

void Connection::appendToRegexFilters (std::string const & str, bool checked, bool inclusive)
{
	m_session_state.appendToRegexFilters(str, checked, inclusive);
}

void Connection::removeFromRegexFilters (std::string const & val)
{
	m_session_state.removeFromRegexFilters(val);
}

void Connection::recompileRegexps ()
{
	for (int i = 0, ie = sessionState().m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex & fr = sessionState().m_filtered_regexps[i];
		QStandardItem * root = m_regex_model->invisibleRootItem();
		QString const qregex = QString::fromStdString(fr.m_regex_str);
		QStandardItem * child = findChildByText(root, qregex);
		fr.m_is_enabled = false;
		if (!child)
			continue;
		QRegExp regex(qregex);
		if (regex.isValid())
		{
			fr.m_regex = regex;
			bool const checked = (child->checkState() == Qt::Checked);
			if (child && checked)
			{
				child->setData(QBrush(Qt::green), Qt::BackgroundRole);
				child->setToolTip(tr("ok"));
				fr.m_is_enabled = true;
			}
			else if (child && !checked)
			{
				child->setData(QBrush(Qt::yellow), Qt::BackgroundRole);
				child->setToolTip(tr("regex not enabled"));
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

void Connection::appendToColorRegexFilters (std::string const & val)
{
	m_session_state.appendToColorRegexFilters(val);
}

void Connection::removeFromColorRegexFilters (std::string const & val)
{
	m_session_state.removeFromColorRegexFilters(val);
}

void Connection::loadToColorRegexps (std::string const & filter_item, std::string const & color, bool enabled)
{
	sessionState().appendToColorRegexFilters(filter_item);
	sessionState().setRegexColor(filter_item, QColor(color.c_str()));
	sessionState().setRegexChecked(filter_item, enabled);
}


void Connection::recompileColorRegexps ()
{
	for (int i = 0, ie = sessionState().m_colorized_texts.size(); i < ie; ++i)
	{
		ColorizedText & ct = sessionState().m_colorized_texts[i];
		QStandardItem * root = m_color_regex_model->invisibleRootItem();
		QString const qregex = QString::fromStdString(ct.m_regex_str);
		QStandardItem * child = findChildByText(root, qregex);
		ct.m_is_enabled = false;
		if (!child)
			continue;
		QRegExp regex(qregex);
		if (regex.isValid())
		{
			ct.m_regex = regex;

			bool const checked = (child->checkState() == Qt::Checked);
			if (child && checked)
			{
				child->setData(QBrush(Qt::green), Qt::BackgroundRole);
				child->setToolTip(tr("ok"));
				ct.m_is_enabled = true;
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

void Connection::loadToRegexps (std::string const & filter_item, bool inclusive, bool enabled)
{
	sessionState().appendToRegexFilters(filter_item, inclusive, enabled);
}

void Connection::flipFilterMode (E_FilterMode mode)
{
	qDebug("filterMode changed: old=%u -> new=%u", sessionState().m_filter_mode, mode);
	if (sessionState().m_filter_mode != mode)
	{
		QStandardItem * node = m_file_model->invisibleRootItem();
		flipCheckState(node);
		flipCheckStateChilds(node);
		sessionState().flipFilterMode(mode);
	}
}

/*void Connection::syncFileTreeWithFilter (E_FilterMode mode, QStandardItem * node)
{
	setCheckState(node, checked);
	int const rc = node->rowCount();
	for (int r = 0; r < rc; ++r)
	{
		QStandardItem * child = node->child(r, 0);
		syncFileTreeWithFilter(child, checked);
	}
}*/

