#include "treemodel.h"

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
    //if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    //    return QString("grr");
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

	qDebug("row count of=%s is %i    r=%i c=%i", node->key.c_str(), count, index.row(), index.column());
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
		{
			//qDebug("indexof=%s row=%i col=%i", child_item->key.c_str(), row, column); 
			return createIndex(row, column, child_item);
		}
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
		//qDebug("role=%i dataof=%s row=%i col=%i", role, item->key.c_str(), index.row(), index.column()); 
		return QVariant(QString::fromStdString(item->key));
		//@TODO: columns: return item->data(index.column());
	}
	else if (role == Qt::UserRole) // collapsed or expanded?
	{
		//qDebug("role=%i dataof=%s row=%i col=%i", role, item->key.c_str(), index.row(), index.column()); 
		return static_cast<bool>(item->data.m_collapsed);
	}
	else if (role == Qt::CheckStateRole)
	{
		//qDebug("role=%i dataof=%s row=%i col=%i", role, item->key.c_str(), index.row(), index.column());
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
	
	node_t * const n = m_tree_data->set_to_state(path, i);
	n->data.m_state = Qt::Checked;
	QModelIndex const idx = indexFromItem(n);
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
		Qt::CheckState const state = static_cast<Qt::CheckState>(value.toInt());
		item->data.m_state = state;

		if (state == Qt::Checked)
		{
			node_t const * child = item->children;
			while (child)
			{
				setData(indexFromItem(child), value, role);
				child = child->next;
			}
		}
		else if (state == Qt::Unchecked)
		{
			node_t const * child = item->children;
			while (child)
			{
				setData(indexFromItem(child), value, role);
				child = child->next;
			}

			//if (item->parent && item->parent->parent)
			//	setData(indexFromItem(item->parent), QVariant(Qt::PartiallyChecked), role);
		}
		else if (state == Qt::PartiallyChecked)
		{
			//if (item->parent && item->parent->parent)
		//		setData(indexFromItem(item->parent), QVariant(Qt::PartiallyChecked), role);
		}
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

QModelIndex TreeModel::hideLinearParents () const
{
	node_t const * node = m_tree_data->root;
	node_t const * child = node->children;
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



#if kurvaprdel
void TreeModel::modelItemChanged (QStandardItem * item)
{
    QStandardItem * parent = item->parent();
    int checkCount = 0;
    int rowCount = parent->rowCount();

    for (int i = 0; i < rowCount; i++)
        if (parent->child(i)->checkState() == Qt::Checked)
            checkCount++;
    switch (checkCount)
    {
		case 0:
			parent->setCheckState(Qt::Unchecked);
			break;
		case rowCount:
			parent->setCheckState(Qt::Checked);
			break;
		default:
			parent->setCheckState(Qt::PartiallyChecked);
    }
}

void TreeModel::ModelItemChanged (QStandardItem * item)
{
    QStandardItem * parent = item->parent();
    if (parent == 0)
	{
        //folder state changed--> update children if not partially selected
        Qt::CheckState newState = item->checkState();
        if(newState != Qt::PartiallyChecked){
            for (int i = 0; i < item->rowCount(); i++)
            {
                item->child(i)->setCheckState(newState);
            }
        }
    }
    else
	{
		//child item changed--> count parent's children that are checked
        int checkCount = 0;
        for (int i = 0; i < parent->rowCount(); i++)
        {
            if (parent->child(i)->checkState() == Qt::Checked)
                checkCount++;
        }

        if(checkCount == 0)
            parent->setCheckState(Qt::Unchecked);
        else if (checkCount ==  parent->rowCount())
            parent->setCheckState(Qt::Checked);
        else
            parent->setCheckState(Qt::PartiallyChecked);
    }
}
#endif
