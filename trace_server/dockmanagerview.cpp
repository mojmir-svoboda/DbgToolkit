#include "dockmanagerview.h"

DockManagerView::DockManagerView (QWidget * parent)
	: TreeView(parent)
{
	setEditTriggers(QAbstractItemView::CurrentChanged);
}

