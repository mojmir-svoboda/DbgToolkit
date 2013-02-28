#include "treemodel.h"
#include <QTreeView>

TreeModel::TreeModel (QObject * parent, tree_data_t * data)
	: QAbstractItemModel(parent)
	, m_tree_data(data)
{
	qDebug("%s", __FUNCTION__);
}

void TreeModel::beforeLoad()
{
	emit layoutAboutToBeChanged();
}

void TreeModel::afterLoad ()
{
	emit layoutChanged();
}

bool TreeModel::hasChildren (QModelIndex const & parent) const
{
	node_t const * parent_node = itemFromIndex(parent);
	return parent_node->children != 0;
}

QModelIndex TreeModel::rootIndex () const
{
	return indexFromItem(m_tree_data->root);
}

TreeModel::~TreeModel () { /* leave m_tree_data untouched. they are owned by sessionState */ }

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

int TreeModel::rowCount (QModelIndex const & index) const
{
	node_t const * const node = itemFromIndex(index);
	int count = 0;
	node_t const * child = node->children;
	while (child)
	{
		++count;
		child = child->next;
	}
	return count;
}

int TreeModel::columnCount (QModelIndex const & parent) const
{
	return 1; // @TODO: not supported yet
}

TreeModel::node_t const * TreeModel::itemFromIndex (QModelIndex const & index) const
{
	if (index.isValid())
		if (node_t const * node = static_cast<node_t const *>(index.internalPointer()))
			return node;
	return m_tree_data->root;
}

TreeModel::node_t * TreeModel::itemFromIndex (QModelIndex const & index)
{
	if (index.isValid())
		if (node_t * node = static_cast<node_t *>(index.internalPointer()))
			return node;
	return m_tree_data->root;
}

QModelIndex TreeModel::index (int row, int column, QModelIndex const & parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	if (node_t * const parent_item = const_cast<node_t *>(itemFromIndex(parent))) // sigh, const_cast
		if (node_t * const child_item = node_t::node_child(parent_item, row))
			return createIndex(row, column, child_item);
	return QModelIndex();
}

QModelIndex TreeModel::parent (QModelIndex const & index) const
{
    if (!index.isValid())
		return QModelIndex();

	node_t const * const item = itemFromIndex(index);
	node_t * const parent_node = item->parent;

    if (parent_node == 0 || parent_node == m_tree_data->root)
		return QModelIndex();

	int const parent_row = parent_node->row;
	int const parent_column = 0;
	QModelIndex const parent = createIndex(parent_row, parent_column, parent_node);
	return parent;
}

QModelIndex TreeModel::indexFromItem (node_t const * item) const
{
	int const row = item->row;
	int const column = 0;

	if (!item->parent)
		return QModelIndex();

	int const parent_row = item->parent->row;
	int const parent_column = 0;
	void * const parent_node = item->parent;
	QModelIndex const parent = createIndex(parent_row, parent_column, parent_node);
	QModelIndex const idx = index(row, column, parent);
	return idx;
}

QVariant TreeModel::data (QModelIndex const & index, int role) const
{
	if (!index.isValid())
		return QVariant();

	node_t const * const item = itemFromIndex(index);
	if (role == Qt::DisplayRole)
	{
		return QVariant(item->key);
	}
	else if (role == Qt::UserRole) // collapsed or expanded?
	{
		return static_cast<bool>(item->data.m_collapsed);
	}
	else if (role == Qt::CheckStateRole)
	{
		return static_cast<Qt::CheckState>(item->data.m_state);
	}
	return QVariant();
}

void TreeModel::onExpanded (QModelIndex const & idx) { setData(idx, 0, Qt::UserRole); }
void TreeModel::onCollapsed (QModelIndex const & idx) { setData(idx, 1, Qt::UserRole); }

void TreeModel::stateToChildren (node_t * item, Qt::CheckState state)
{
	TreeModel::node_t * child = item->children;
	while (child)
	{
		child->data.m_state = state;
		QModelIndex ci = indexFromItem(child);
		emit dataChanged(ci, ci);
		stateToChildren(child, state);
		child = child->next;
	}
}

void TreeModel::stateToParents (node_t * item, Qt::CheckState state)
{
	node_t * parent = item->parent;
	while (parent)
	{
		parent->data.m_state = state;
		QModelIndex pi = indexFromItem(parent);
		emit dataChanged(pi, pi);
		parent = parent->parent;
	}
}

void TreeModel::syncParents (node_t * const item, Qt::CheckState state)
{
	if (!item) return;
	node_t * parent = item->parent;
	if (!parent) return;
	node_t * child = parent->children;
	if (!child) return;

	while (child)
	{
		if (child != item)
		{
			bool const same_state = state == child->data.m_state;
			if (!same_state)
				return;
		}
		child = child->next;
	}

	parent->data.m_state = state;
	QModelIndex pi = indexFromItem(parent);
	emit dataChanged(pi, pi);
	
	syncParents(parent, state);
}

bool TreeModel::setData (QModelIndex const & index, QVariant const & value, int role)
{
	if (!index.isValid()) return false;

	node_t * const item = itemFromIndex(index);
	if (role == Qt::DisplayRole)
		return false;
	else if (role == Qt::UserRole)
	{
		bool const collapsed = value.toBool();
		item->data.m_collapsed = collapsed;
	}
	else if (role == Qt::CheckStateRole)
	{
		Qt::CheckState const state = static_cast<Qt::CheckState>(value.toInt());
		item->data.m_state = state;

		if (state == Qt::Checked)
		{
			stateToChildren(item, state);
			stateToParents(item, Qt::PartiallyChecked);
			syncParents(item, state);
		}
		else if (state == Qt::Unchecked)
		{
			stateToChildren(item, state);
			stateToParents(item, Qt::PartiallyChecked);
			syncParents(item, state);
		}
		else if (state == Qt::PartiallyChecked)
		{ }
		emit invalidateFilter(); // @TODO: perf problem - do not call recursively!
	}
	else
		return false;

	emit dataChanged(index, index);
	return true;
}

Qt::ItemFlags TreeModel::flags (QModelIndex const & index) const
{
	return QAbstractItemModel::flags(index)
				| Qt::ItemIsEnabled
				| Qt::ItemIsUserCheckable
			//	| Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled
				| Qt::ItemIsSelectable
				| Qt::ItemIsTristate;
}

bool TreeModel::insertColumns (int position, int columns, QModelIndex const & parent)
{
    bool success = true;
    beginInsertColumns(parent, position, position + columns - 1);
    endInsertColumns();
    return success;
}

bool TreeModel::insertRows (int position, int rows, QModelIndex const & parent)
{
    node_t * parent_item = itemFromIndex(parent);
    bool success = true;
    beginInsertRows(parent, position, position + rows - 1);
    endInsertRows();
    return success;
}

QModelIndex TreeModel::hideLinearParents () const
{
	node_t const * node = m_tree_data->root;
	node_t const * child = node;
	if (!child)
		return QModelIndex();
	node_t const * last_linear = child;
	while (child)
	{
		size_t const child_count = child->count_childs();
		if (child_count == 1)
		{
			last_linear = child;
			child = child->children;
			continue;
		}
		else
			break;
	}
	return indexFromItem(last_linear);
}

void TreeModel::syncExpandState (QTreeView * tv)
{
	if (!m_tree_data->root->children)
		return;
	QList<node_t const *> q;
	q.push_back(m_tree_data->root->children);
	while (!q.empty())
	{
		node_t const * n = q.back();
		bool const expanded = !static_cast<bool>(n->data.m_collapsed);
		tv->setExpanded(indexFromItem(n), expanded);
		q.pop_back();

		node_t const * child = n->children;
		while (child)
		{
			q.push_back(child);
			child = child->next;
		}
	}
}

QModelIndex TreeModel::insertItemWithHint (QString const & path, bool checked)
{
	TreeModelItem const * i = 0;
	node_t const * node = m_tree_data->is_present(path, i);
	if (node)
	{
		//qDebug("%s path=%s already present", __FUNCTION__, path.toStdString().c_str());
		return QModelIndex();
	}
	else
	{
		//qDebug("%s path=%s not present, adding", __FUNCTION__, path.toStdString().c_str());
		TreeModelItem i;
		i.m_state = checked ? Qt::Checked : Qt::Unchecked;
		i.m_collapsed = false;
	
		node_t * const n = m_tree_data->set_to_state(path, i);

		QModelIndex const idx = indexFromItem(n);
		setData(idx, checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
		return idx;
	}
}

QModelIndex TreeModel::insertItem (QString const & path)
{
	TreeModelItem const * i = 0;
	node_t const * node = m_tree_data->is_present(path, i);
	if (node)
	{
		//QModelIndex const idx = indexFromItem(node);
		//emit dataChanged(idx, idx);
		return QModelIndex();
	}
	else
	{
		TreeModelItem i;
		i.m_state = Qt::Checked;
		i.m_collapsed = false;
	
		node_t * const n = m_tree_data->set_to_state(path, i);
		node_t * parent = n->parent;
		while (parent)
		{
			QModelIndex pi = indexFromItem(parent);
			emit dataChanged(pi, pi);
			parent = parent->parent;
		}

		if (n->parent)
		{
			n->data.m_state = n->parent->data.m_state == Qt::Checked ? Qt::Checked : Qt::Unchecked;
		}
		else
			n->data.m_state = Qt::Checked;
		QModelIndex const idx = indexFromItem(n);
		emit dataChanged(idx, idx);
		return idx;
	}
}

QModelIndex TreeModel::stateToItem (QString const & path, Qt::CheckState state, bool collapsed)
{
	TreeModelItem const * i;
	node_t const * const node = m_tree_data->is_present(path, i);
	if (!node)
		return QModelIndex();

	QModelIndex const idx = indexFromItem(node);
	setData(idx, state, Qt::CheckStateRole);
	setData(idx, collapsed,   Qt::UserRole);
	return idx;
}

QModelIndex TreeModel::stateToItem (QString const & path, Qt::CheckState state)
{
	TreeModelItem const * i;
	node_t const * const node = m_tree_data->is_present(path, i);
	if (!node)
		return QModelIndex();

	QModelIndex const idx = indexFromItem(node);
	setData(idx, state, Qt::CheckStateRole);
	return idx;
}

QModelIndex TreeModel::selectItem (QTreeView * tv, QString const & path)
{
	TreeModelItem const * i = 0;
	node_t const * const node = m_tree_data->is_present(path, i);
	if (!node)
		return QModelIndex();

	QModelIndex const idx = indexFromItem(node);
	tv->setCurrentIndex(idx);
	return idx;
}


