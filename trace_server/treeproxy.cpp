#include "treeproxy.h"
#include "connection.h"
#include "mainwindow.h"

TreeProxyModel::TreeProxyModel (QTreeView * parent)
	: KSelectionProxyModel(parent->selectionModel())
{
}
