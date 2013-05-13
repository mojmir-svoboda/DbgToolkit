#include "tableview.h"
#include <QEvent>
#include <QHelpEvent>
#include <QHeaderView>
#include "modelview.h"

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
    int bottom = model()->rowCount(rootIndex()) - 1;
    // make sure that bottom is the bottommost *visible* row
    while (bottom >= 0 && isRowHidden(verticalHeader()->logicalIndex(bottom)))
        --bottom;

    int right = model()->columnCount(rootIndex()) - 1;

    while (right >= 0 && isColumnHidden(horizontalHeader()->logicalIndex(right)))
        --right;

    if (bottom == -1 || right == -1)
        return QModelIndex(); // model is empty

    QModelIndex current = currentIndex();

    if (!current.isValid()) {
        int row = 0;
        int column = 0;
        while (column < right && isColumnHidden(horizontalHeader()->logicalIndex(column)))
            ++column;
        while (isRowHidden(verticalHeader()->logicalIndex(row)) && row < bottom)
            ++row;
        d->visualCursor = QPoint(column, row);
        return model()->index(verticalHeader()->logicalIndex(row), horizontalHeader()->logicalIndex(column), rootIndex());
    }

    // Update visual cursor if current index has changed.
    QPoint visualCurrent(d->visualColumn(current.column()), d->visualRow(current.row()));
    if (visualCurrent != d->visualCursor) {
        if (d->hasSpans()) {
            QSpanCollection::Span span = d->span(current.row(), current.column());
            if (span.top() > d->visualCursor.y() || d->visualCursor.y() > span.bottom()
                || span.left() > d->visualCursor.x() || d->visualCursor.x() > span.right())
                d->visualCursor = visualCurrent;
        } else {
            d->visualCursor = visualCurrent;
        }
    }

    int visualRow = d->visualCursor.y();
    if (visualRow > bottom)
        visualRow = bottom;
    Q_ASSERT(visualRow != -1);
    int visualColumn = d->visualCursor.x();
    if (visualColumn > right)
        visualColumn = right;
    Q_ASSERT(visualColumn != -1);

    if (isRightToLeft()) {
        if (cursorAction == MoveLeft)
            cursorAction = MoveRight;
        else if (cursorAction == MoveRight)
            cursorAction = MoveLeft;
    }

    switch (cursorAction) {
    case MoveUp: {
        int originalRow = visualRow;
#ifdef QT_KEYPAD_NAVIGATION
        if (QApplication::keypadNavigationEnabled() && visualRow == 0)
            visualRow = d->visualRow(model()->rowCount() - 1) + 1;
            // FIXME? visualRow = bottom + 1;
#endif
        int r = verticalHeader()->logicalIndex(visualRow);
        int c = horizontalHeader()->logicalIndex(visualColumn);
        if (r != -1 && d->hasSpans()) {
            QSpanCollection::Span span = d->span(r, c);
            if (span.width() > 1 || span.height() > 1)
                visualRow = d->visualRow(span.top());
        }
        while (visualRow >= 0) {
            --visualRow;
            r = verticalHeader()->logicalIndex(visualRow);
            c = horizontalHeader()->logicalIndex(visualColumn);
            if (r == -1 || (!isRowHidden(r) && d->isCellEnabled(r, c)))
                break;
        }
        if (visualRow < 0)
            visualRow = originalRow;
        break;
    }
    case MoveDown: {
        int originalRow = visualRow;
        if (d->hasSpans()) {
            QSpanCollection::Span span = d->span(current.row(), current.column());
            visualRow = d->visualRow(d->rowSpanEndLogical(span.top(), span.height()));
        }
#ifdef QT_KEYPAD_NAVIGATION
        if (QApplication::keypadNavigationEnabled() && visualRow >= bottom)
            visualRow = -1;
#endif
        int r = verticalHeader()->logicalIndex(visualRow);
        int c = horizontalHeader()->logicalIndex(visualColumn);
        if (r != -1 && d->hasSpans()) {
            QSpanCollection::Span span = d->span(r, c);
            if (span.width() > 1 || span.height() > 1)
                visualRow = d->visualRow(d->rowSpanEndLogical(span.top(), span.height()));
        }
        while (visualRow <= bottom) {
            ++visualRow;
            r = verticalHeader()->logicalIndex(visualRow);
            c = horizontalHeader()->logicalIndex(visualColumn);
            if (r == -1 || (!isRowHidden(r) && d->isCellEnabled(r, c)))
                break;
        }
        if (visualRow > bottom)
            visualRow = originalRow;
        break;
    }
    case MovePrevious:
    case MoveLeft: {
        int originalRow = visualRow;
        int originalColumn = visualColumn;
        bool firstTime = true;
        bool looped = false;
        bool wrapped = false;
        do {
            int r = verticalHeader()->logicalIndex(visualRow);
            int c = horizontalHeader()->logicalIndex(visualColumn);
            if (firstTime && c != -1 && d->hasSpans()) {
                firstTime = false;
                QSpanCollection::Span span = d->span(r, c);
                if (span.width() > 1 || span.height() > 1)
                    visualColumn = d->visualColumn(span.left());
            }
            while (visualColumn >= 0) {
                --visualColumn;
                r = verticalHeader()->logicalIndex(visualRow);
                c = horizontalHeader()->logicalIndex(visualColumn);
                if (r == -1 || c == -1 || (!isRowHidden(r) && !isColumnHidden(c) && d->isCellEnabled(r, c)))
                    break;
                if (wrapped && (originalRow < visualRow || (originalRow == visualRow && originalColumn <= visualColumn))) {
                    looped = true;
                    break;
                }
            }
            if (cursorAction == MoveLeft || visualColumn >= 0)
                break;
            visualColumn = right + 1;
            if (visualRow == 0) {
                wrapped = true;
                visualRow = bottom;
            } else {
                --visualRow;
            }
        } while (!looped);
        if (visualColumn < 0)
            visualColumn = originalColumn;
        break;
    }
    case MoveNext:
    case MoveRight: {
        int originalRow = visualRow;
        int originalColumn = visualColumn;
        bool firstTime = true;
        bool looped = false;
        bool wrapped = false;
        do {
            int r = verticalHeader()->logicalIndex(visualRow);
            int c = horizontalHeader()->logicalIndex(visualColumn);
            if (firstTime && c != -1 && d->hasSpans()) {
                firstTime = false;
                QSpanCollection::Span span = d->span(r, c);
                if (span.width() > 1 || span.height() > 1)
                    visualColumn = d->visualColumn(d->columnSpanEndLogical(span.left(), span.width()));
            }
            while (visualColumn <= right) {
                ++visualColumn;
                r = verticalHeader()->logicalIndex(visualRow);
                c = horizontalHeader()->logicalIndex(visualColumn);
                if (r == -1 || c == -1 || (!isRowHidden(r) && !isColumnHidden(c) && d->isCellEnabled(r, c)))
                    break;
                if (wrapped && (originalRow > visualRow || (originalRow == visualRow && originalColumn >= visualColumn))) {
                    looped = true;
                    break;
                }
            }
            if (cursorAction == MoveRight || visualColumn <= right)
                break;
            visualColumn = -1;
            if (visualRow == bottom) {
                wrapped = true;
                visualRow = 0;
            } else {
                ++visualRow;
            }
        } while (!looped);
        if (visualColumn > right)
            visualColumn = originalColumn;
        break;
    }
    case MoveHome:
        visualColumn = 0;
        while (visualColumn < right && d->isVisualColumnHiddenOrDisabled(visualRow, visualColumn))
            ++visualColumn;
        if (modifiers & Qt::ControlModifier) {
            visualRow = 0;
            while (visualRow < bottom && d->isVisualRowHiddenOrDisabled(visualRow, visualColumn))
                ++visualRow;
        }
        break;
    case MoveEnd:
        visualColumn = right;
        if (modifiers & Qt::ControlModifier)
            visualRow = bottom;
        break;
    case MovePageUp: {
        int newRow = rowAt(visualRect(current).top() - d->viewport->height());
        if (newRow == -1)
            newRow = verticalHeader()->logicalIndex(0);
        return model()->index(newRow, current.column(), rootIndex());
    }
    case MovePageDown: {
        int newRow = rowAt(visualRect(current).bottom() + d->viewport->height());
        if (newRow == -1)
            newRow = verticalHeader()->logicalIndex(bottom);
        return model()->index(newRow, current.column(), rootIndex());
    }}

    d->visualCursor = QPoint(visualColumn, visualRow);
    int logicalRow = verticalHeader()->logicalIndex(visualRow);
    int logicalColumn = horizontalHeader()->logicalIndex(visualColumn);
    if (!model()->hasIndex(logicalRow, logicalColumn, rootIndex()))
        return QModelIndex();

    QModelIndex result = model()->index(logicalRow, logicalColumn, rootIndex());
    if (!d->isRowHidden(logicalRow) && !d->isColumnHidden(logicalColumn) && d->isIndexEnabled(result))
        return result;

    return QModelIndex();
}

