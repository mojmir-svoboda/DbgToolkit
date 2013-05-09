#include "treeproxy.h"
#include "connection.h"
#include "mainwindow.h"

TreeProxyModel::TreeProxyModel (QTreeView * parent)
	: KSelectionProxyModel(parent->selectionModel())
{
}


void TreeProxyModel::setFindString (QString const & s)
{
	m_find = s;
	beginResetModel();
	endResetModel();
}
