#include "treemodel.h"

TreeModel::TreeModel (QObject * parent, tree_data_t * data)
	: QAbstractItemModel(parent)
	, m_tree_data(data)
{
}

TreeModel::~TreeModel ()
{
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QString("grr");//rootItem->data(section);
    return QVariant();
}


int TreeModel::rowCount (QModelIndex const & parent) const
{
	//qDebug("{ %s for parent.row=%i parent.col=%i parent.ptr=%i", __FUNCTION__, parent.row(), parent.column(), parent.internalPointer());
	if (parent.isValid())
	{
		node_t const * parent_node = static_cast<node_t const *>(parent.internalPointer());
		int count = 0;
		node_t const * child = parent_node->children;
		while (child)
		{
			++count;
			child = child->next;
		}
		qDebug("p.r=%u count=%u", parent.row(), count);
		return count;
	}
	return 0;
}

int TreeModel::columnCount (QModelIndex const & parent) const
{
	//qDebug("{ %s for parent.row=%i parent.col=%i parent.ptr=%i", __FUNCTION__, parent.row(), parent.column(), parent.internalPointer());
	return 1; // @TODO: not supported yet
}

TreeModel::node_t const * TreeModel::itemFromIndex (QModelIndex const & index) const
{
	return static_cast<node_t const *>(index.internalPointer());
}

QModelIndex TreeModel::index (int row, int column, QModelIndex const & parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	node_t * const parent_item = static_cast<node_t *>(parent.internalPointer());
	if (parent_item)
	{
		if (node_t * const child_item = node_t::node_child(parent_item, row))
			return createIndex(row, column, child_item);
	}
	return QModelIndex();

}

QModelIndex TreeModel::parent (QModelIndex const & index) const
{
    if (!index.isValid()) return QModelIndex();

	node_t const * const item = static_cast<node_t const *>(index.internalPointer());
	if (!item || item->parent)
		return QModelIndex();

	node_t * const parent_node = item->parent;
	int const parent_row = parent_node->row;
	int const parent_column = 0;

    if (parent_node == m_tree_data->root)
		return QModelIndex();

	QModelIndex const parent = createIndex(parent_row, parent_column, parent_node);
	return parent;
}

TreeModel::node_t * TreeModel::itemFromIndex (QModelIndex const & index)
{
	return static_cast<node_t *>(index.internalPointer());
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
	qDebug("{ %s", __FUNCTION__);
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
		// 
	}
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
	int const orig_count = n->parent ? n->parent->count_childs() - 1: 0;
	QModelIndex idx = indexFromItem(n);

	//beginInsertRows(parent(idx), 0, 20);
	beginInsertRows(idx, 0, 2);
	qDebug("+++ %s", path.c_str());

	emit dataChanged(idx, idx);
	endInsertRows();
}

bool TreeModel::selectItem (std::string const & s)
{
	return true;
}

bool TreeModel::setData (QModelIndex const & index, QVariant const & value, int role)
{
	qDebug("{ %s", __FUNCTION__);
	if (!index.isValid())
		return false;

	node_t * const item = itemFromIndex(index);

	//@TODO: column bool result = item->setData(index.column(), value);

	if (role == Qt::DisplayRole)
		return false;
	else if (role == Qt::UserRole)
	{
		bool const collapsed = value.toBool();
		item->data.m_collapsed = collapsed;
	}
	else if (role == Qt::CheckStateRole)
	{
		int state = value.toInt();
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
				| Qt::ItemIsDragEnabled
				| Qt::ItemIsDropEnabled
				| Qt::ItemIsSelectable
				| Qt::ItemIsTristate;
}

