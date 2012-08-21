#include "treeview.h"

TreeView::TreeView (QWidget * parent)
	: QTreeView(parent)
{
	setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void TreeView::setModel (TreeModel * model)
{
	if (m_current == model) return;

	disconnect(this, SIGNAL(     expanded(QModelIndex const &)), model, SLOT(onExpanded(QModelIndex const &)));
	disconnect(this, SIGNAL(    collapsed(QModelIndex const &)), model, SLOT(onCollapsed(QModelIndex const &)));
	disconnect(this, SIGNAL(      clicked(QModelIndex const &)), model, SLOT(onClicked(QModelIndex const &)));
	disconnect(this, SIGNAL(doubleClicked(QModelIndex const &)), model, SLOT(onDblClicked(QModelIndex const &)));

	if (!m_models.contains(model))
		m_models.push_back(model);
	m_current = model;
	QTreeView::setModel(model);

	connect(this, SIGNAL(     expanded(QModelIndex const &)), model, SLOT(onExpanded(QModelIndex const &)));
	connect(this, SIGNAL(    collapsed(QModelIndex const &)), model, SLOT(onCollapsed(QModelIndex const &)));
	connect(this, SIGNAL(      clicked(QModelIndex const &)), model, SLOT(onClicked(QModelIndex const &)));
	connect(this, SIGNAL(doubleClicked(QModelIndex const &)), model, SLOT(onDblClicked(QModelIndex const &)));

	hideLinearParents();
}

void TreeView::hideLinearParents ()
{
	setRootIndex(m_current->hideLinearParents());
}


#if defined NE_E
/// W specific

	m_main_window->getWidgetFile()->setEnabled(m_main_window->filterEnabled());

			bool const orig_exp = m_main_window->getWidgetFile()->isExpanded(m_file_model->indexFromItem(node));

	//m_main_window->getWidgetFile()->setEnabled(m_main_window->filterEnabled());
}

/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void syncOnPreset (QStandardItem * qnode, file_filter<FilteredFile>::node_t * node)
{
	if (node)
	{
		qnode->setCheckState(static_cast<Qt::CheckState>(node->data.m_state));
		if (node && node->children)
		{
			file_filter<FilteredFile>::node_t * child = node->children;
			while (child)
			{
				QStandardItem * qchild = findChildByText(qnode, QString::fromStdString(child->key));
				if (!qchild)
				{
					QList<QStandardItem *> row_items = addRowTriState(QString::fromStdString(child->key), static_cast<E_NodeStates>(child->data.m_state));
					qnode->appendRow(row_items);
					qchild = row_items[0];
				}
				syncOnPreset(qchild, child);
				child = child->next;
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
void MainWindow::onPresetActivate (Connection * conn, QString const & pname)
{
	if (!conn) return;

	SessionState dummy;
	if (loadSession(dummy, pname))
	{
		std::swap(conn->m_session_state.m_file_filters.root, dummy.m_file_filters.root);
		conn->m_session_state.m_filtered_regexps = dummy.m_filtered_regexps;

		file_filter<FilteredFile>::node_t * node = conn->m_session_state.m_file_filters.root;
		QStandardItem * qnode = static_cast<QStandardItemModel *>(getWidgetFile()->model())->invisibleRootItem();
		syncOnPreset(qnode, node);

	}
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
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
}
/////////////////////////////////////////////////////////////////////////////////////
void Connection::appendToFileTree (boost::char_separator<char> const & sep, std::string const & fileline, bool exclude)
{
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
	tokenizer_t tok(fileline, sep);

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
			FilteredFile const * fi = 0;
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
			FilteredFile ff;
			bool const known = sessionState().m_file_filters.is_present(path, ff);
			if (known)
			{
				ff_state = static_cast<E_NodeStates>(ff.m_state);
				node->setCheckState(static_cast<Qt::CheckState>(ff.m_state));
				new_state = ff_state;
			}
			else
			{
				new_state = e_Checked;
				//qDebug("unknown: %s, state=%u", path.c_str(), new_state);
				sessionState().m_file_filters.set_to_state(fileline, static_cast<E_NodeStates>(new_state));
			}

			bool const orig_exp = m_main_window->getWidgetFile()->isExpanded(m_file_model->indexFromItem(node));
			//qDebug("new node: %s, state=%u", path.c_str(), new_state);
			QList<QStandardItem *> row_items = addRowTriState(qItem, new_state);
			node->appendRow(row_items);

			if (ff_state == e_PartialCheck)
			{
				m_main_window->getWidgetFile()->setExpanded(m_file_model->indexFromItem(node), orig_exp);
			}

			node = row_items.at(0);
		}
		++it;

		if (it == ite)
		{
			if (exclude)
			{
				E_NodeStates new_state = e_Checked;
				sessionState().m_file_filters.set_to_state(fileline, static_cast<E_NodeStates>(new_state));
				node->setCheckState(static_cast<Qt::CheckState>(new_state));
			}

			////////////

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
				set_state_to_topdown(sessionState().m_file_filters, fileline, static_cast<E_NodeStates>(curr_state), e_PartialCheck);
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
/////////////////////////////////////////////////////////////////////////////////////
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

	sessionState().m_file_filters.set_to_state(file, FilteredFile(static_cast<E_NodeStates>(node->checkState()), collapsed));
}

void Connection::onFileExpanded (QModelIndex const & idx)
{
	onFileColOrExp(idx, false);
}

void Connection::onFileCollapsed (QModelIndex const & idx)
{
	onFileColOrExp(idx, true);
}

/////////////////////////////////////////////////////////////////////////////////////

	QObject::connect(getWidgetFile(), SIGNAL(expanded(QModelIndex const &)), connection, SLOT(onFileExpanded(QModelIndex const &)));
	QObject::connect(getWidgetFile(), SIGNAL(collapsed(QModelIndex const &)), connection, SLOT(onFileCollapsed(QModelIndex const &)));
/////////////////////////////////////////////////////////////////////////////////////
void Connection::setupModelFile ()
{
	if (!m_file_model)
	{
		qDebug("new tree view file model");
		m_file_model = new QStandardItemModel;
	}
	m_main_window->getWidgetFile()->setModel(m_file_model);
	m_main_window->getWidgetFile()->expandAll();
	m_main_window->getWidgetFile()->setEnabled(m_main_window->filterEnabled());
}

/////////////////////////////////////////////////////////////////////////////////////
	getWidgetFile()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidgetFile(), SIGNAL(clicked(QModelIndex)), m_server, SLOT(onClickedAtFileTree(QModelIndex)));
	connect(getWidgetFile(), SIGNAL(doubleClicked(QModelIndex)), m_server, SLOT(onDoubleClickedAtFileTree(QModelIndex)));

/////////////////////////////////////////////////////////////////////////////////////
	m_main_window->getWidgetFile()->setModel(m_file_model);


/////////////////////////////////////////////////////////////////////////////////////
	std::vector<QString> s;	// @TODO: hey piggy, to member variables

void Server::onClickedAtFileTree_Impl (QModelIndex idx, bool recursive)
{
	Connection * const conn = findCurrentConnection();
	if (!conn)
		return;

	QStandardItemModel const * const model = static_cast<QStandardItemModel *>(getWidgetFile()->model());
	QStandardItem * const node = model->itemFromIndex(idx);
	QStandardItem const * line_node = 0;

	s.clear();
	s.reserve(16);
	if (!node->hasChildren())
		line_node = node;
	else
		s.push_back(model->data(idx, Qt::DisplayRole).toString());

	QStandardItem * parent = node->parent();
	std::string file;
	QModelIndex parent_idx = model->indexFromItem(parent);
	while (parent_idx.isValid())
	{
		QString const & val = model->data(parent_idx, Qt::DisplayRole).toString();
		s.push_back(val);
		parent = parent->parent();
		parent_idx = model->indexFromItem(parent);
	}

	for (std::vector<QString>::const_reverse_iterator it=s.rbegin(), ite=s.rend(); it != ite; ++it)
		file += std::string("/") + (*it).toStdString();

	fileline_t filter_node(file, std::string());
	if (line_node)
	{
		QString const & val = model->data(idx, Qt::DisplayRole).toString();
		filter_node.second = val.toStdString();
	}
	std::string const fileline = filter_node.first + "/" + filter_node.second;

	Qt::CheckState const curr_state = node->checkState();
	if (curr_state == Qt::Checked)
	{
		// unchecked --> checked
		setCheckStateChilds(node, curr_state);
		conn->sessionState().m_file_filters.set_state_to_childs(fileline, static_cast<E_NodeStates>(curr_state));

		QStandardItem * p = node;
		while (p = p->parent())
		{
			bool const all_checked = checkChildState(p, Qt::Checked);
			if (all_checked)
			{
				//conn->sessionState().m_file_filters.set_state_to_childs(fileline, static_cast<E_NodeStates>(curr_state));
				p->setCheckState(Qt::Checked);
			}
		}
	}
	else if (curr_state == Qt::Unchecked)
	{
		// checked --> unchecked
		set_state_to_topdown(conn->sessionState().m_file_filters, fileline, static_cast<E_NodeStates>(curr_state), e_PartialCheck);
		setCheckStateChilds(node, curr_state);
		setCheckStateReverse(node->parent(), Qt::PartiallyChecked); // iff parent unchecked and clicked on leaf
	}

	E_NodeStates const new_state = static_cast<E_NodeStates>(curr_state);

	qDebug("file click! sync state of %s --> node_checkstate=%i", fileline.c_str(), node->checkState());
	conn->sessionState().m_file_filters.set_to_state(fileline, static_cast<E_NodeStates>(new_state));
	conn->onInvalidateFilter();
}

/////////////////////////////////////////////////////////////////////////////////////

void Server::onClickedAtFileTree (QModelIndex idx)
{
	onClickedAtFileTree_Impl(idx, false);
}

/////////////////////////////////////////////////////////////////////////////////////

void Server::onDoubleClickedAtFileTree (QModelIndex idx)
{
	//QStandardItemModel * model = static_cast<QStandardItemModel *>(getWidgetFile()->model());
	//QStandardItem * item = model->itemFromIndex(idx);
	//onClickedAtFileTree_Impl(idx, true);
}

#endif
