#pragma once
#include <QTreeView>
#include <QStandardItemModel>
#include <QList>
#include "TreeModel.h"

class TreeView : public QTreeView
{
	Q_OBJECT
public:

	TreeView (QWidget * parent = 0);

	void setModel (TreeModel * m);
	void setHidingLinearParents (bool state) { m_hiding = state; }

	void hideLinearParents ();
	void syncExpandState ();

protected:
	QList<TreeModel *> m_models;
	TreeModel * m_current;
	bool m_hiding;
};


