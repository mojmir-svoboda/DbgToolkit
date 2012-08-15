#include "treemodel.h"

int TreeModel::rowCount (QModelIndex const & parent = QModelIndex()) const
{
}

int TreeModel::columnCount (QModelIndex const & parent = QModelIndex()) const
{
}

QVariant TreeModel::data (const QModelIndex & index, int role = Qt::DisplayRole) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DisplayRole)
	{
	}
	else if (role == Qt::UserRole)
	{
		// collapsed or expanded?
	}
	else if (role == Qt::CheckStateRole)
	{
		// 
	}
}

bool TreeModel::setData (QModelIndex const & index, QVariant const & value, int role = Qt::EditRole)
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DisplayRole)
	{
	}
	else if (role == Qt::UserRole)
	{
		bool const collapsed = value.toBool();

		// collapsed or expanded?
	}
	else if (role == Qt::CheckStateRole)
	{
		// 
	}
}

Qt::ItemFlags flags (QModelIndex const & index) const
{
	return QStandardItemModel::flags(index) | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsTristate;
}

