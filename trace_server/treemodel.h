//#include <QStandardItemModel>
#include <QAbstractItemModel>
#include <filters/file_filter.hpp>
#include "config.h"

typedef tree_filter<TreeViewItem> tree_data_t;

class TreeModel : public QStandardItemModel
{
	Q_OBJECT

	tree_data_t * m_tree_data;
public:

	typedef QStandardItemModel parent_t;
	explicit TreeModel (QObject * parent = 0, tree_data_t * data = 0);
	~ModelView ();

	virtual QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const;
	virtual bool setData (QModelIndex const & index, QVariant const & value, int role = Qt::EditRole);
	//virtual QVariant headerData (int section, Qt::Orientation orientation, int role) const;

	//virtual QModelIndex index (int row, int column, const QModelIndex &parent = QModelIndex()) const;
	//virtual QModelIndex parent (QModelIndex const & child) const;

	virtual int rowCount (QModelIndex const & parent = QModelIndex()) const;
	virtual int columnCount (QModelIndex const & parent = QModelIndex()) const;

	//virtual bool insertRows (int row, int count, QModelIndex const &);
	//virtual bool insertColumns (int column, int count, const QModelIndex &parent = QModelIndex());
	virtual Qt::ItemFlags flags (QModelIndex const & index) const;
};

