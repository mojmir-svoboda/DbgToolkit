#include "treeview.h"
#include "treemodel.h"

TreeView::TreeView (QWidget * parent)
	: QTreeView(parent)
	, m_hiding(true)
{
	setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void TreeView::setModel (QAbstractItemModel * model)
{
	if (m_current == model) return;

	disconnect(this, SIGNAL(     expanded(QModelIndex const &)), model, SLOT(onExpanded(QModelIndex const &)));
	disconnect(this, SIGNAL(    collapsed(QModelIndex const &)), model, SLOT(onCollapsed(QModelIndex const &)));
	//disconnect(this, SIGNAL(      clicked(QModelIndex const &)), model, SLOT(onClicked(QModelIndex const &)));
	//disconnect(this, SIGNAL(doubleClicked(QModelIndex const &)), model, SLOT(onDblClicked(QModelIndex const &)));

	if (!m_models.contains(model))
		m_models.push_back(model);
	m_current = model;
	QTreeView::setModel(model);

	connect(this, SIGNAL(     expanded(QModelIndex const &)), model, SLOT(onExpanded(QModelIndex const &)));
	connect(this, SIGNAL(    collapsed(QModelIndex const &)), model, SLOT(onCollapsed(QModelIndex const &)));
	//connect(this, SIGNAL(      clicked(QModelIndex const &)), model, SLOT(onClicked(QModelIndex const &)));
	//connect(this, SIGNAL(doubleClicked(QModelIndex const &)), model, SLOT(onDblClicked(QModelIndex const &)));

	hideLinearParents();
}

void TreeView::unsetModel (QAbstractItemModel * model)
{
	if (m_current == model)
	{
		m_current = 0;
		QTreeView::setModel(0);
	}

	disconnect(this, SIGNAL(     expanded(QModelIndex const &)), model, SLOT(onExpanded(QModelIndex const &)));
	disconnect(this, SIGNAL(    collapsed(QModelIndex const &)), model, SLOT(onCollapsed(QModelIndex const &)));
	//disconnect(this, SIGNAL(      clicked(QModelIndex const &)), model, SLOT(onClicked(QModelIndex const &)));
	//disconnect(this, SIGNAL(doubleClicked(QModelIndex const &)), model, SLOT(onDblClicked(QModelIndex const &)));

	if (int const idx = m_models.indexOf(model) >= 0)
		m_models.removeAt(idx);
}

void TreeView::hideLinearParents ()
{
	if (m_hiding && m_current)
	{
		//if (TreeModel * model = qobject_cast<TreeModel *>(m_current))
		//	setRootIndex(static_cast<TreeModel *>(m_current)->hideLinearParents());
	}
}

void TreeView::syncExpandState ()
{
	//if (TreeModel * model = qobject_cast<TreeModel *>(m_current))
	//	static_cast<TreeModel *>(m_current)->syncExpandState(this);
}

