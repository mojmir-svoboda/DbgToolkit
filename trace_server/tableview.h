#pragma once
#include <QTableView>

class SessionState;

class TableView : public QTableView
{
public:
	explicit TableView (QWidget * parent = 0);
	virtual ~TableView ();

	void setColumnOrder (QMap<int, int> const & columnOrderMap, SessionState const & s);

	virtual void scrollTo (QModelIndex const & index, ScrollHint hint = EnsureVisible);

protected:
	virtual void keyPressEvent (QKeyEvent * event);
    virtual QModelIndex moveCursor (CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
	virtual bool viewportEvent (QEvent * event);
};
