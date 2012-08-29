#include <QTableView>

class TableView : public QTableView
{
public:
	explicit TableView (QWidget * parent = 0) : QTableView(parent) { }
	bool viewportEvent (QEvent * event);
};
