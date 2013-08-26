#include "treemodel.h"
#include <QTreeView>

template <class InfoT>
TreeModel<InfoT>::TreeModel (QObject * parent, tree_data_t * data)
	: QAbstractItemModel(parent)
	, m_tree_data(data)
	, m_cut_parent_lvl(0)
{
	qDebug("%s", __FUNCTION__);
}

template <class InfoT>
void TreeModel<InfoT>::beforeLoad()
{
	emit layoutAboutToBeChanged();
}

template <class InfoT>
void TreeModel<InfoT>::afterLoad ()
{
	emit layoutChanged();
}

template <class InfoT>
bool TreeModel<InfoT>::hasChildren (QModelIndex const & parent) const
{
	node_t const * parent_node = itemFromIndex(parent);
	return parent_node->children != 0;
}

template <class InfoT>
QModelIndex TreeModel<InfoT>::rootIndex () const
{
	return indexFromItem(m_tree_data->root);
}

template <class InfoT>
TreeModel<InfoT>::~TreeModel () { /* leave m_tree_data untouched. they are owned by sessionState */ }

template <class InfoT>
QVariant TreeModel<InfoT>::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

template <class InfoT>
int TreeModel<InfoT>::rowCount (QModelIndex const & index) const
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

template <class InfoT>
int TreeModel<InfoT>::columnCount (QModelIndex const & parent) const
{
	return 1; // @TODO: not supported yet
}

template <class InfoT>
typename TreeModel<InfoT>::node_t const * TreeModel<InfoT>::itemFromIndex (QModelIndex const & index) const
{
	if (index.isValid())
		if (node_t const * node = static_cast<node_t const *>(index.internalPointer()))
			return node;
	return m_tree_data->root;
}

template <class InfoT>
typename TreeModel<InfoT>::node_t * TreeModel<InfoT>::itemFromIndex (QModelIndex const & index)
{
	if (index.isValid())
		if (node_t * node = static_cast<node_t *>(index.internalPointer()))
			return node;
	return m_tree_data->root;
}

template <class InfoT>
QModelIndex TreeModel<InfoT>::index (int row, int column, QModelIndex const & parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	if (node_t * const parent_item = const_cast<node_t *>(itemFromIndex(parent))) // sigh, const_cast
		if (node_t * const child_item = node_t::node_child(parent_item, row))
			return createIndex(row, column, child_item);
	return QModelIndex();
}

template <class InfoT>
QModelIndex TreeModel<InfoT>::parent (QModelIndex const & index) const
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

template <class InfoT>
QModelIndex TreeModel<InfoT>::indexFromItem (node_t const * item) const
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

template <class InfoT>
QVariant TreeModel<InfoT>::data (QModelIndex const & index, int role) const
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

template <class InfoT>
void TreeModel<InfoT>::expanded (QModelIndex const & idx) { setData(idx, 0, Qt::UserRole); }
template <class InfoT>
void TreeModel<InfoT>::collapsed (QModelIndex const & idx) { setData(idx, 1, Qt::UserRole); }

template <class InfoT>
void TreeModel<InfoT>::stateToChildren (node_t * item, Qt::CheckState state)
{
	TreeModel<InfoT>::node_t * child = item->children;
	while (child)
	{
		child->data.m_state = state;
		QModelIndex ci = indexFromItem(child);
		emit dataChanged(ci, ci);
		stateToChildren(child, state);
		child = child->next;
	}
}

template <class InfoT>
void TreeModel<InfoT>::stateToParents (node_t * item, Qt::CheckState state)
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

template <class InfoT>
void TreeModel<InfoT>::syncParents (node_t * const item, Qt::CheckState state)
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

template <class InfoT>
bool TreeModel<InfoT>::setData (QModelIndex const & index, QVariant const & value, int role)
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
		//emit invalidateFilter(); // @TODO: perf problem - do not call recursively!
	}
	else
		return false;

	emit dataChanged(index, index);
	return true;
}

template <class InfoT>
Qt::ItemFlags TreeModel<InfoT>::flags (QModelIndex const & index) const
{
	return QAbstractItemModel::flags(index)
				| Qt::ItemIsEnabled
				| Qt::ItemIsUserCheckable
			//	| Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled
				| Qt::ItemIsSelectable
				| Qt::ItemIsTristate;
}

template <class InfoT>
bool TreeModel<InfoT>::insertColumns (int position, int columns, QModelIndex const & parent)
{
    bool success = true;
    beginInsertColumns(parent, position, position + columns - 1);
    endInsertColumns();
    return success;
}

template <class InfoT>
bool TreeModel<InfoT>::insertRows (int position, int rows, QModelIndex const & parent)
{
    node_t * parent_item = itemFromIndex(parent);
    bool success = true;
    beginInsertRows(parent, position, position + rows - 1);
    endInsertRows();
    return success;
}

template <class InfoT>
void TreeModel<InfoT>::onCutParentValueChanged (int i)
{
	m_cut_parent_lvl = i;
}

template <class InfoT>
QModelIndex TreeModel<InfoT>::hideLinearParents () const
{
	node_t const * node = m_tree_data->root;
	node_t const * child = node;
	if (!child)
		return QModelIndex();
	node_t const * last_linear = child;
	unsigned level = 0;
	while (child)
	{
		size_t const child_count = child->count_childs();
		if (child_count == 1)
		{
			last_linear = child;
			child = child->children;

			if (m_cut_parent_lvl > 0 && ++level >= m_cut_parent_lvl)
				break;
			else
				continue;
		}
		else
		{
			if (m_cut_parent_lvl > 0)
			{
				last_linear = child;
				child = child->children;

				if (++level >= m_cut_parent_lvl)
					break;
				else
					continue;
			}
			else
				break;
		}
	}
	return indexFromItem(last_linear);
}

template <class InfoT>
void TreeModel<InfoT>::syncExpandState (QTreeView * tv)
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

template <class InfoT>
QModelIndex TreeModel<InfoT>::insertItemWithHint (QString const & path, bool checked)
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

template <class InfoT>
void const * TreeModel<InfoT>::insertItem (QString const & path)
{
	TreeModelItem const * i = 0;
	node_t const * node = m_tree_data->is_present(path, i);
	if (node)
	{
		//QModelIndex const idx = indexFromItem(node);
		//emit dataChanged(idx, idx);
		//return QModelIndex();
		return 0;
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
		return n;
	}
}

template <class InfoT>
QModelIndex TreeModel<InfoT>::stateToItem (QString const & path, Qt::CheckState state, bool collapsed)
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

template <class InfoT>
QModelIndex TreeModel<InfoT>::stateToItem (QString const & path, Qt::CheckState state)
{
	TreeModelItem const * i;
	node_t const * const node = m_tree_data->is_present(path, i);
	if (!node)
		return QModelIndex();

	QModelIndex const idx = indexFromItem(node);
	setData(idx, state, Qt::CheckStateRole);
	return idx;
}

template <class InfoT>
QModelIndex TreeModel<InfoT>::selectItem (QTreeView * tv, QString const & path, bool scroll_to)
{
	TreeModelItem const * i = 0;
	node_t const * const node = m_tree_data->is_present(path, i);
	if (!node)
		return QModelIndex();

	QModelIndex const idx = indexFromItem(node);
	tv->setCurrentIndex(idx);
	if (scroll_to)
		tv->scrollTo(idx, QAbstractItemView::PositionAtCenter);
	return idx;
}

template <class InfoT>
void TreeModel<InfoT>::expandParents (QTreeView * tv, node_t * item, bool state)
{
	node_t * parent = item->parent;
	while (parent)
	{
		parent->data.m_collapsed = !state;
		QModelIndex pi = indexFromItem(parent);
		//emit dataChanged(pi, pi);
		tv->setExpanded(pi, state);
		
		parent = parent->parent;
	}
}

template <class InfoT>
QModelIndex TreeModel<InfoT>::expandItem (QTreeView * tv, QString const & path)
{
	TreeModelItem const * i = 0;
	node_t const * const node = m_tree_data->is_present(path, i);
	if (!node)
		return QModelIndex();

	QModelIndex const idx = indexFromItem(node);
	tv->setExpanded(idx, true);

	node_t * const grr = const_cast<node_t *>(node); // @FIXME is_present does not have mutable version
	expandParents(tv, grr, true);

	return idx;
}

template <class InfoT>
QModelIndexList TreeModel<InfoT>::find (QString const & s) const
{
	QModelIndexList children;

	for (int r = 0; r < rowCount(); ++r )
		children << index(r, 0);

	for ( int i = 0; i < children.size(); ++i ) {
		for ( int j = 0; j < rowCount(children[i]); ++j ) {
			children << children[i].child(j, 0);
		}
	}

	QModelIndexList matches;
	for ( int i = 0; i < children.size(); ++i )
		matches << match(children[i], Qt::DisplayRole, s);

	return matches;
}

template <class InfoT>
void TreeModel<InfoT>::collapseChilds (QTreeView * tv)
{
	QModelIndexList children;
	for (int r = 0; r < rowCount(); ++r)
		children << index(r, 0);

	for (int i = 0; i < children.size(); ++i) {
		for (int j = 0; j < rowCount(children[i]); ++j) {
			children << children[i].child(j, 0);
		}
	}

	for (int i = 0; i < children.size(); ++i )
	{
		int const rows = rowCount(children.at(i));
		QModelIndex const & parent = children.at(i);
		bool has_children = false;
		for (int r = 0; r < rows; ++r)
		{
			QModelIndex const & children_idx = parent.child(r, 0);
			has_children |= hasChildren(children_idx);
		}

		if (!has_children && rows > 0)
		{
			QString aa = data(parent, Qt::DisplayRole).toString();
			qDebug("collapse: %s", aa.toStdString().c_str());
			setData(parent, true,   Qt::UserRole);
			tv->setExpanded(parent, false);
		}
	}
}

