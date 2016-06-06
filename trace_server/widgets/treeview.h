#pragma once
#include <QTreeView>
#include <QStandardItemModel>
#include <QList>

class TreeView : public QTreeView
{
	Q_OBJECT
public:

	TreeView (QWidget * parent = 0);
	virtual ~TreeView ();

	void setModel (QAbstractItemModel * m);
	void unsetModel (QAbstractItemModel * m);
	void setHidingLinearParents (bool state) { m_hiding = state; }

	void hideLinearParents ();
	void syncExpandState ();

protected:
	QList<QAbstractItemModel *> m_models;
	QAbstractItemModel * m_current;
	bool m_hiding;
};


