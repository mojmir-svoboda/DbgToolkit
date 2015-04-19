#pragma once
#include <QTableView>

class SessionState;

class TableView : public QTableView
{
public:
	explicit TableView (QWidget * parent = 0);
	virtual ~TableView ();

	virtual void showWarningSign () = 0;

	//void setColumnOrder (QMap<int, int> const & columnOrderMap, SessionState const & s);

protected:
	virtual bool viewportEvent (QEvent * event);
};
