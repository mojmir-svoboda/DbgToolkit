#include <QTableView>

class TableView : public QTableView
{
public:
	explicit TableView (QWidget * parent = 0);
	virtual ~TableView ();
	bool viewportEvent (QEvent * event);
};
