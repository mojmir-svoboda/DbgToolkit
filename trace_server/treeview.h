#include <QTreeView>
#include <QStandardItemModel>
#include <QList>
#include "TreeModel.h"

class TreeView : public QTreeView
{
	Q_OBJECT
public:

	TreeView (QObject * parent = 0);

	void setModel (TreeModel * m);

Q_SIGNALS:
    void expanded (QModelIndex const & idx);
    void collapsed (QModelIndex const & idx);

public Q_SLOTS:
    void expand (QModelIndex const & idx);
    void collapse (QModelIndex const & idx);

protected:
	QList<TreeModel *> m_models;
	TreeModel * m_current;
};


