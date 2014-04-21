#include "dockmanagermodel.h"
#include <QString>
#include <QMainWindow>
#include "dockmanager.h"
#include "dock.h"

DockManagerModel::DockManagerModel (QObject * parent, tree_data_t * data)
	: TreeModel<DockedInfo>(parent, data)
{
	qDebug("%s", __FUNCTION__);
}

DockManagerModel::~DockManagerModel ()
{
	qDebug("%s", __FUNCTION__);
}


QModelIndex DockManagerModel::insertItemWithPath (QStringList const & path, bool checked)
{
	QString const name = path.join("/");
	DockedInfo const * i = 0;
	node_t const * node = m_tree_data->is_present(name, i);
	if (node)
	{
		//qDebug("%s path=%s already present", __FUNCTION__, path.toStdString().c_str());
		return QModelIndex();
	}
	else
	{
		//qDebug("%s path=%s not present, adding", __FUNCTION__, path.toStdString().c_str());
		DockedInfo i;
		i.m_state = checked ? Qt::Checked : Qt::Unchecked;
		i.m_collapsed = false;
		//i.m_path = path;
	
		node_t * const n = m_tree_data->set_to_state(name, i);
		QModelIndex const parent_idx = indexFromItem(n->parent);
		beginInsertRows(parent_idx, 0, n->parent->count_childs() - 1);
		n->data.m_path = path;

		QModelIndex const idx = indexFromItem(n);
		endInsertRows();
		setData(idx, checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
		//QModelIndex const parent_idx = idx.parent();
		//if (parent_idx.isValid())
		//	emit dataChanged(parent_idx, parent_idx);
		return idx;
	}
}

int DockManagerModel::columnCount (QModelIndex const & parent) const
{
	return 2; // @TODO: not supported yet
}

Qt::ItemFlags DockManagerModel::flags (QModelIndex const & index) const
{
	return QAbstractItemModel::flags(index)
				| Qt::ItemIsEnabled
				| Qt::ItemIsUserCheckable
			//	| Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled
				| Qt::ItemIsSelectable
				| Qt::ItemIsTristate;
}


DockedWidgetBase const * DockManagerModel::getWidgetFromIndex (QModelIndex const & index) const
{
	DockManager const * const mgr = static_cast<DockManager const *>(QObject::parent());
	node_t const * const item = itemFromIndex(index);
	QStringList const & p = item->data.m_path;
	DockedWidgetBase const * const dwb = mgr->findDockable(p.join("/"));
	return dwb;
}

DockedWidgetBase * DockManagerModel::getWidgetFromIndex (QModelIndex const & index)
{
	DockManager * const mgr = static_cast<DockManager *>(QObject::parent());
	node_t const * const item = itemFromIndex(index);
	QStringList const & p = item->data.m_path;
	DockedWidgetBase * dwb = mgr->findDockable(p.join("/"));
	return dwb;
}

QVariant DockManagerModel::data (QModelIndex const & index, int role) const
{
	if (!index.isValid())
		return QVariant();
	
	int const col = index.column();

	if (col == e_Visibility)
		return TreeModel<DockedInfo>::data(index, role);

	if ((col == e_SyncGroup && role == Qt::DisplayRole) || role == e_DockRoleSyncGroup)
	{
		if (DockedWidgetBase const * const dwb = getWidgetFromIndex(index))
			return QVariant(dwb->dockedConfig().m_sync_group);
	}
	if ((col == e_InCentralWidget && role == Qt::DisplayRole) || role == e_DockRoleCentralWidget)
	{
		if (DockedWidgetBase const * const dwb = getWidgetFromIndex(index))
			return QVariant(dwb->dockedConfig().m_central_widget);
	}


	/*if (col == e_SyncGroup && role == Qt::EditRole)
	}*/


	/*node_t const * const item = itemFromIndex(index);
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
	}*/
	return QVariant();
}

bool DockManagerModel::setData (QModelIndex const & index, QVariant const & value, int role)
{
	if (!index.isValid()) return false;

	node_t * const item = itemFromIndex(index);
	int const col = index.column();

	if (col == e_SyncGroup && role == Qt::EditRole)
	{
		if (DockedWidgetBase * const dwb = getWidgetFromIndex(index))
		{
			int const sg = value.toInt();
			dwb->dockedConfig().m_sync_group = sg;
		}
	}
	else if (role <= Qt::UserRole)
	{
		return TreeModel<DockedInfo>::setData(index, value, role);
	}
	else if (role == e_DockRoleCentralWidget)
	{
		if (DockedWidgetBase * const dwb = getWidgetFromIndex(index))
		{
			int const on = value.toInt();
			dwb->dockedConfig().m_central_widget = on;
		}
	}
	else if (role == e_DockRoleSyncGroup)
	{
		if (DockedWidgetBase * const dwb = getWidgetFromIndex(index))
		{
			int const sg = value.toInt();
			dwb->dockedConfig().m_sync_group = sg;
		}
	}
	else if (role == e_DockRoleSelect)
	{
	}
	else
		return false;

	emit dataChanged(index, index);
	return true;
}




