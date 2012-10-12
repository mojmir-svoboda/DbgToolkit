#include <QTableView>

class TableView : public QTableView
{
public:
	explicit TableView (QWidget * parent = 0);
	virtual ~TableView ();
	virtual bool viewportEvent (QEvent * event);
	virtual void scrollTo (QModelIndex const & index, ScrollHint hint = EnsureVisible);
};
