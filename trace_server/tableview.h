#pragma once
#include <QTableView>

class SessionState;

class TableView : public QTableView
{
public:
	explicit TableView (QWidget * parent = 0);
	virtual ~TableView ();

	//void setColumnOrder (QMap<int, int> const & columnOrderMap, SessionState const & s);

protected:
	virtual bool viewportEvent (QEvent * event);
};
