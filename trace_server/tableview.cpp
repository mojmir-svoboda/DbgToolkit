#include "tableview.h"
#include <QEvent>
#include <QHelpEvent>
#include <QHeaderView>
#include <QScrollBar>
#include "modelview.h"

TableView::TableView (QWidget * parent)
	: QTableView(parent)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	//setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
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

void TableView::setColumnOrder (QMap<int, int> const & columnOrderMap, SessionState const & session)
{
    ModelView * tableModel = static_cast<ModelView *>(model());
    int currentIndex;
    int indexMovingTo;

    QMapIterator<int, int> iter(columnOrderMap);
    while (iter.hasNext())
    {
        iter.next();
        currentIndex  = horizontalHeader()->visualIndex(session.findColumn4Tag(iter.key()));
        indexMovingTo = iter.value();

        if(indexMovingTo <= 0)
        	continue;

       	horizontalHeader()->moveSection(currentIndex, indexMovingTo);
    }
}

void TableView::keyPressEvent (QKeyEvent * event)
{
	QTableView::keyPressEvent(event);
}


QModelIndex TableView::moveCursor (CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
	if (modifiers & Qt::ControlModifier)
	{
		if (cursorAction == MoveHome)
		{
			scrollToTop();
			return QModelIndex(); // @FIXME: should return valid value
		}
		else if (cursorAction == MoveEnd)
		{
			scrollToBottom();
			return QModelIndex();	// @FIXME too
		}
		else
			return QTableView::moveCursor(cursorAction, modifiers);
	}
	else
		return QTableView::moveCursor(cursorAction, modifiers);

	/*int const value = horizontalScrollBar()->value();
	QModelIndex const ret = QTableView::moveCursor(cursorAction, modifiers);
	horizontalScrollBar()->setValue(value);
	return ret;*/
}

