#include <QTreeView>
#include <QStandardItemModel>


class TreeModel : public QStandardItemModel
{


};

class TreeView : public QTreeView
{
	Q_OBJECT
public:
	TreeView (QObject *parent = 0);



};

