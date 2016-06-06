#include "treeproxy.h"

TreeProxyModel::TreeProxyModel (TreeModel<TreeModelItem> * src, QItemSelectionModel * selection)
	: KSelectionProxyModel(selection)
	, m_selection(selection)
	, m_src(src)
{
}


void TreeProxyModel::setFindString (QString const & s)
{
	m_find = s;

	QModelIndexList const children = m_src->find(s);

	m_selection->clear();
	for (int i = 0, ie = children.size(); i < ie; ++i)
		m_selection->select(children.at(i), QItemSelectionModel::Select);
	//beginResetModel();
	//endResetModel();
}
