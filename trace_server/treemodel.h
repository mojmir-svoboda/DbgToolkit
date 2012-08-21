#pragma once
#include <QAbstractItemModel>
#include "config.h"
#include <filters/file_filter.hpp>

class QTreeView;
typedef tree_filter<TreeModelItem> tree_data_t;

class TreeModel : public QAbstractItemModel
{
	Q_OBJECT
public:

	typedef QAbstractItemModel parent_t;
	typedef tree_data_t::node_t node_t;

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
	//bool removeColumns (int position, int columns, QModelIndex const & parent = QModelIndex());
	bool insertRows (int position, int rows, QModelIndex const & parent = QModelIndex());
	//bool removeRows (int position, int rows, QModelIndex const & parent = QModelIndex());
	//
	bool hasChildren (QModelIndex const & parent = QModelIndex()) const;

	QModelIndex hideLinearParents () const;

	bool insertItem (std::string const & s);
	bool selectItem (std::string const & s);

	void beforeLoad ();
	void afterLoad ();
	QModelIndex rootIndex () const;

	void syncExpandState (QTreeView *);
	void selectItem (QTreeView *, std::string const &);

public Q_SLOTS:
	
	//void onItemChanged (QModelIndex const & idx);
	void onExpanded (QModelIndex const & idx);
	void onCollapsed (QModelIndex const & idx);

protected:

	node_t const * itemFromIndex (QModelIndex const & index) const;
	node_t * itemFromIndex (QModelIndex const & index);
	QModelIndex indexFromItem (node_t const * item) const;

	tree_data_t * m_tree_data;
};

