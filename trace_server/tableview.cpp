#include "tableview.h"
#include <QEvent>
#include <QHelpEvent>

TableView::TableView (QWidget * parent)
	: QTableView(parent)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

TableView::~TableView () 
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

bool TableView::viewportEvent (QEvent * event)
{
	if (event->type() == QEvent::ToolTip)
	{
		QHelpEvent * he = static_cast<QHelpEvent *>(event);
		QModelIndex index = indexAt(he->pos());
		QRect vr = visualRect(index);
		QSize shr = itemDelegate(index)->sizeHint(viewOptions(), index);
		if (shr.width() > vr.width())
			return QTableView::viewportEvent(event);
		else
			return false;
	}
	else
		return QTableView::viewportEvent(event);
}

void TableView::scrollTo (QModelIndex const & index, ScrollHint hint)
{
	QTableView::scrollTo(index, hint);
}
