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
void Connection::onClearCurrentRegexFilter ()
{
	sessionState().onClearRegexFilter();
	onInvalidateFilter();
}
void Connection::onClearCurrentScopeFilter ()
{
	sessionState().onClearScopeFilter();
	onInvalidateFilter();
}

void Connection::onExcludeFileLine (QModelIndex const & row_index)
{
	QString file = findString4Tag(tlv::tag_file, row_index);
	QString line = findString4Tag(tlv::tag_line, row_index);

	fileline_t filter_item(file.toStdString(), line.toStdString());
	qDebug("appending: %s:%s", file.toStdString().c_str(), line.toStdString().c_str());
	//m_session_state.appendFileFilter(filter_item);
	//bool const checked = m_main_window->fltMode() == e_Exclude ? Qt::checked : false;
	boost::char_separator<char> sep(":/\\");
	appendToFileTree(sep, file.toStdString() + "/" + line.toStdString(), true);

	onInvalidateFilter();
}

void Connection::appendToFileTree (boost::char_separator<char> const & sep, std::string const & fileline, bool exclude)
{
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
	tokenizer_t tok(fileline, sep);

	E_FilterMode const fmode = m_main_window->fltMode();
	QStandardItem * node = m_file_model->invisibleRootItem();
	QStandardItem * last_hidden_node = 0;
	bool stop = false;
	std::string path;
	tokenizer_t::const_iterator it = tok.begin(), ite = tok.end();
	while (it != ite)
	{
		QString qItem = QString::fromStdString(*it);
		QStandardItem * child = findChildByText(node, qItem);
		path += "/";
		path += *it;
		if (child != 0)
		{
			node = child;

			E_NodeStates ff_state = e_Unchecked;
			bool col_state = true;
			file_info const * fi = 0;
			bool const known = sessionState().m_file_filters.is_present(path, fi);
			if (known)
			{
				node->setCheckState(static_cast<Qt::CheckState>(fi->m_state));
				m_main_window->getWidgetFile()->setExpanded(m_file_model->indexFromItem(node), !fi->m_collapsed);
			}

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
				//qDebug("unknown: %s, state=%u", path.c_str(), new_state);
				sessionState().m_file_filters.set_to_state(fileline, static_cast<E_NodeStates>(new_state));
			}

			//qDebug("new node: %s, state=%u", path.c_str(), new_state);
			QList<QStandardItem *> row_items = addRowTriState(qItem, new_state);
			node->appendRow(row_items);
			node = row_items.at(0);
		}
		++it;

		if (it == ite)
		{
			if (exclude)
			{
				E_NodeStates new_state = e_Checked;
				new_state = fmode == e_Include ? e_Unchecked : e_Checked;
				sessionState().m_file_filters.set_to_state(fileline, static_cast<E_NodeStates>(new_state));
				node->setCheckState(static_cast<Qt::CheckState>(new_state));
			}

			////////////

			E_FilterMode const fmode = m_main_window->fltMode();
			Qt::CheckState const curr_state = node->checkState();
			if (curr_state == Qt::Checked)
			{
				// unchecked --> checked
				setCheckStateChilds(node, curr_state);
				sessionState().m_file_filters.set_state_to_childs(fileline, static_cast<E_NodeStates>(curr_state));

				if (node->parent())
				{
					//@TODO progress up the tree (reverse)
					bool const all_checked = checkChildState(node->parent(), Qt::Checked);
					if (all_checked && node->parent())
						node->parent()->setCheckState(Qt::Checked);
				}
			}
			else if (curr_state == Qt::Unchecked)
			{
				// checked --> unchecked
				sessionState().m_file_filters.set_state_to_topdown(fileline, static_cast<E_NodeStates>(curr_state), e_PartialCheck);
				setCheckStateChilds(node, curr_state);
				setCheckStateReverse(node->parent(), Qt::PartiallyChecked); // iff parent unchecked and clicked on leaf
			}

			E_NodeStates const new_state = static_cast<E_NodeStates>(curr_state);

			//qDebug("file click! sync state of %s --> node_checkstate=%i", fileline.c_str(), node->checkState());
			sessionState().m_file_filters.set_to_state(fileline, static_cast<E_NodeStates>(new_state));

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


void Connection::onFileColOrExp (QModelIndex const & idx, bool collapsed)
{
	QStandardItemModel const * const model = static_cast<QStandardItemModel *>(m_main_window->getWidgetFile()->model());
	QStandardItem * const node = model->itemFromIndex(idx);

	std::vector<QString> s;	// @TODO: hey piggy, to member variables
	s.clear();
	s.reserve(16);
	QStandardItem * parent = node;
	QModelIndex parent_idx = model->indexFromItem(parent);
	while (parent_idx.isValid())
	{
		QString const & val = model->data(parent_idx, Qt::DisplayRole).toString();
		s.push_back(val);
		parent = parent->parent();
		parent_idx = model->indexFromItem(parent);
	}

	std::string file;
	for (std::vector<QString>::const_reverse_iterator it=s.rbegin(), ite=s.rend(); it != ite; ++it)
		file += std::string("/") + (*it).toStdString();

	sessionState().m_file_filters.set_to_state(file, static_cast<E_NodeStates>(node->checkState()), false);
}

void Connection::onFileExpanded (QModelIndex const & idx)
{
	qDebug("%s", __FUNCTION__);
	onFileColOrExp(idx, false);
}

void Connection::onFileCollapsed (QModelIndex const & idx)
{
	qDebug("%s", __FUNCTION__);
	onFileColOrExp(idx, true);
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

