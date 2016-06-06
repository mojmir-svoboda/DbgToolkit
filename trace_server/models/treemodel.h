#pragma once
#include <QAbstractItemModel>
#include "config.h"
#include <filters/file_filter.hpp>

class QTreeView;
class Connection;

template <class InfoT>
class TreeModel : public QAbstractItemModel
{
public:

	typedef QAbstractItemModel parent_t;
	typedef tree_filter<InfoT> tree_data_t;
	typedef typename tree_data_t::node_t node_t;

	explicit TreeModel (QObject * parent = 0, tree_data_t * data = 0);
	~TreeModel ();

	virtual QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const;
	virtual bool setData (QModelIndex const & index, QVariant const & value, int role = Qt::EditRole);
	virtual QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	//bool setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role = Qt::EditRole);

	virtual QModelIndex index (int row, int column, QModelIndex const & parent = QModelIndex()) const;
	virtual QModelIndex parent (QModelIndex const & child) const;

	virtual int rowCount (QModelIndex const & parent = QModelIndex()) const;
	virtual int columnCount (QModelIndex const & parent = QModelIndex()) const;

	virtual Qt::ItemFlags flags (QModelIndex const & index) const;

	bool insertColumns (int position, int columns, QModelIndex const & parent = QModelIndex());
	bool insertRows (int position, int rows, QModelIndex const & parent = QModelIndex());
	bool hasChildren (QModelIndex const & parent = QModelIndex()) const;

	QModelIndex hideLinearParents () const;

	void const * insertItem (QString const & s);
	QModelIndex insertItemWithHint (QString const & s, bool checked);

	void beforeLoad ();
	void afterLoad ();
	QModelIndex rootIndex () const;

	void syncExpandState (QTreeView *);
	QModelIndex selectItem (QTreeView *, QString const &, bool scroll_to);
	QModelIndex expandItem (QTreeView *, QString const &);
	QModelIndex stateToItem (QString const & path, Qt::CheckState state);
	QModelIndex stateToItem (QString const & path, Qt::CheckState state, bool collapsed);

	QModelIndexList find (QString const & s) const;

	void onCutParentValueChanged (int i);
	void collapseChilds (QTreeView * tv);
	void stateToChildren (node_t * item, Qt::CheckState state);

	void expanded (QModelIndex const & idx);
	void collapsed (QModelIndex const & idx);

protected:
	node_t const * itemFromIndex (QModelIndex const & index) const;
	node_t * itemFromIndex (QModelIndex const & index);
	QModelIndex indexFromItem (node_t const * item) const;

	void stateToParents (node_t * item, Qt::CheckState state);
	void expandParents (QTreeView * tv, node_t * item, bool state);
	void syncParents (node_t * const item, Qt::CheckState state);

	tree_data_t * m_tree_data;
	Connection * m_connection;
	int m_cut_parent_lvl;
};

#include "treemodel.inl"
