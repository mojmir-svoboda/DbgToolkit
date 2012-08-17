#include "treemodel.h"

TreeModel::TreeModel (QObject * parent, tree_data_t * data)
	: QAbstractItemModel(parent)
	, m_tree_data(data)
{ }

TreeModel::~TreeModel () { /* leave m_tree_data untouched. they are owned by sessionState */ }

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    //if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    //    return QString("grr");
    return QVariant();
}

int TreeModel::rowCount (QModelIndex const & parent) const
{
	node_t const * const parent_node = itemFromIndex(parent);
	int count = 0;
	node_t const * child = parent_node->children;
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

    if (parent_node == m_tree_data->root)
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
		return QVariant(QString::fromStdString(item->key));
		//@TODO: columns: return item->data(index.column());
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

void TreeModel::onExpanded (QModelIndex const & idx)
{
	setData(idx, 0, Qt::UserRole);
}

void TreeModel::onCollapsed (QModelIndex const & idx)
{
	setData(idx, 1, Qt::UserRole);
}

bool TreeModel::insertItem (std::string const & path)
{
	TreeModelItem i;
	bool const present = m_tree_data->is_present(path, i);
	if (present)
		return false;
	
	node_t const * const n = m_tree_data->set_to_state(path, i);
	QModelIndex idx = indexFromItem(n);

	emit dataChanged(idx, idx);
}

bool TreeModel::selectItem (std::string const & s)
{
	return true; // @TODO
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
		int const state = value.toInt();
		item->data.m_state = state;
	}
	else
		return false;

	emit dataChanged(index, index);
}

Qt::ItemFlags TreeModel::flags (QModelIndex const & index) const
{
	return QAbstractItemModel::flags(index)
				| Qt::ItemIsEnabled
				| Qt::ItemIsUserCheckable
				| Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled
				| Qt::ItemIsSelectable
				| Qt::ItemIsTristate;
}


bool TreeModel::insertColumns (int position, int columns, QModelIndex const & parent)
{
    bool success = true;
    beginInsertColumns(parent, position, position + columns - 1);
    //success = rootItem->insertColumns(position, columns);
    endInsertColumns();
    return success;
}

bool TreeModel::insertRows (int position, int rows, QModelIndex const & parent)
{
    node_t * parent_item = itemFromIndex(parent);
    bool success = true;
    beginInsertRows(parent, position, position + rows - 1);
    //success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();
    return success;
}

