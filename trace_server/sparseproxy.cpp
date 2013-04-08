#include "sparseproxy.h"
#include "connection.h"
#include "mainwindow.h"

SparseProxyModel::SparseProxyModel (QObject * parent)
	: QAbstractProxyModel(parent)
	, m_main_window(static_cast<Connection const *>(parent)->getMainWindow())
{ }

void SparseProxyModel::force_update ()
{
	emit layoutAboutToBeChanged();

	m_map_from_tgt.clear();
	m_map_from_src.clear();
	QAbstractTableModel const * src_model = static_cast<QAbstractTableModel const *>(sourceModel());
	m_map_from_src.resize(src_model->rowCount());
	for (size_t i = 0; i < src_model->rowCount(); ++i)
		m_map_from_src[i] = -1;

	for (size_t src_idx = 0, se = src_model->rowCount(); src_idx < se; ++src_idx)
	{
		if (filterAcceptsRow(src_idx, QModelIndex()))
		{
			int const tgt_idx = m_map_from_tgt.size();
			m_map_from_tgt.push_back(src_idx);
			m_map_from_src[src_idx] = tgt_idx;
		}
	}

	m_cmap_from_tgt.clear();
	m_cmap_from_src.clear();
	m_cmap_from_src.resize(src_model->columnCount());
	for (size_t i = 0; i < src_model->columnCount(); ++i)
		m_cmap_from_src[i] = -1;
	for (size_t src_idx = 0, se = src_model->columnCount(); src_idx < se; ++src_idx)
	{
		if (filterAcceptsColumn(src_idx, QModelIndex()))
		{
			int const tgt_idx = m_cmap_from_tgt.size();
			m_cmap_from_tgt.push_back(src_idx);
			m_cmap_from_src[src_idx] = tgt_idx;
		}
	}


	emit layoutChanged();
}

QVariant SparseProxyModel::data (QModelIndex const & index, int role) const
{
	QModelIndex const src_idx = mapToSource(index);
	return sourceModel()->data(src_idx, role);
}

bool SparseProxyModel::setData (QModelIndex const & src_index, QVariant const & value, int role)
{
	QModelIndex const index = mapFromSource(src_index);
	if (!index.isValid()) return false;

	emit dataChanged(index, index);
	return true;
}


/*QVariant SparseProxyModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	int const tgt_idx = colFromSource(section);
	return sourceModel()->headerData(tgt_idx, orientation, role);
}

bool  SparseProxyModel::setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role)
{
	int const tgt_idx = colFromSource(section);
	return sourceModel()->setHeaderData(tgt_idx, orientation, value, role);
}*/


Qt::ItemFlags SparseProxyModel::flags (QModelIndex const & index) const
{
	return sourceModel()->flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex SparseProxyModel::index (int row, int column, QModelIndex const & parent) const
{
	if (row < static_cast<int>(m_map_from_tgt.size())
		&& column < static_cast<int>(m_cmap_from_tgt.size()))
		return QAbstractItemModel::createIndex(row, column);
	return QModelIndex();
}
QModelIndex SparseProxyModel::parent (QModelIndex const & child) const
{
	return QModelIndex();
}

int SparseProxyModel::rowCount (QModelIndex const & parent) const
{
	return m_map_from_tgt.size();
}

int SparseProxyModel::columnCount (QModelIndex const & parent) const
{
	return m_cmap_from_tgt.size();
}

bool SparseProxyModel::rowInProxy (int row) const
{
	if (row < static_cast<int>(m_map_from_src.size()))
			return m_map_from_src[row] >= 0;
	return false;
}

bool SparseProxyModel::colInProxy (int col) const
{
	return colToSource(col) > -1;
}

int SparseProxyModel::colToSource (int col) const
{
	int val = -1;
	if (col < static_cast<int>(m_cmap_from_src.size()))
			return m_cmap_from_src[col];
	return val;
}

int SparseProxyModel::colFromSource (int col) const
{
	int val = -1;
	if (col < static_cast<int>(m_cmap_from_tgt.size()))
			return m_cmap_from_tgt[col];
	return val;
}

QModelIndex SparseProxyModel::mapToSource (QModelIndex const & proxyIndex) const
{
	if (proxyIndex.isValid())
		if (proxyIndex.row() < static_cast<int>(m_map_from_tgt.size()))
			if (proxyIndex.column() < static_cast<int>(m_cmap_from_tgt.size()))
			return QAbstractItemModel::createIndex(m_map_from_tgt[proxyIndex.row()], m_cmap_from_tgt[proxyIndex.column()]);
	return QModelIndex();
}

QModelIndex SparseProxyModel::mapFromSource (QModelIndex const & sourceIndex) const
{
	if (sourceIndex.isValid() && sourceIndex.row() < static_cast<int>(m_map_from_src.size())
			&& sourceIndex.column() < static_cast<int>(m_cmap_from_src.size()))
	{
		//qDebug("FPM: %s src.row=%i, src.sz=%u", __FUNCTION__, sourceIndex.row(), m_map_from_src.size());
		return QAbstractItemModel::createIndex(m_map_from_src[sourceIndex.row()], m_cmap_from_src[sourceIndex.column()]);
	}
	return QModelIndex();
}

QModelIndex SparseProxyModel::mapNearestFromSource (QModelIndex const & sourceIndex) const
{
	if (sourceIndex.isValid() && sourceIndex.row() < static_cast<int>(m_map_from_src.size())
			&& sourceIndex.column() < static_cast<int>(m_cmap_from_src.size()))
	{
		if (m_cmap_from_src[sourceIndex.column()] == -1) // column is not in filtered view, finding nearest
		{
			int nearest_bwd_col = -1;
			int dist_bwd = 0;
			for (int d = 0, c = sourceIndex.column(); c --> 0; ++d)
				if (m_cmap_from_src[c] != -1)
				{
					nearest_bwd_col = c;
					dist_bwd = d;
				}

			int nearest_fwd_col = -1;
			int dist_fwd = 0;
			for (int d = 0, c = sourceIndex.column(); c < m_cmap_from_src.size(); ++c, ++d)
				if (m_cmap_from_src[c] != -1)
				{
					nearest_fwd_col = c;
					dist_fwd = d;
				}

			int const res[] = { dist_bwd, dist_fwd };
			int const better_idx = dist_bwd < dist_fwd ? 0 : 1;
			return QAbstractItemModel::createIndex(m_map_from_src[sourceIndex.row()], res[better_idx]);
		}
		//qDebug("FPM: %s src.row=%i, src.sz=%u", __FUNCTION__, sourceIndex.row(), m_map_from_src.size());
		return QAbstractItemModel::createIndex(m_map_from_src[sourceIndex.row()], m_cmap_from_src[sourceIndex.column()]);
	}
	return QModelIndex();
}


bool SparseProxyModel::insertRows (int first, int last, QModelIndex const & parent)
{
	int src_first = -1;
	int src_last = -1;
	for (int r = first; r < last + 1; ++r)
	{
		int const src_idx = m_map_from_src.size();
		int const tgt_idx = m_map_from_tgt.size();
		m_map_from_src.push_back(-1);
		if (filterAcceptsRow(r, QModelIndex()))
		{
			if (src_first == -1)
				src_first = src_idx;
			src_last = src_idx;
			m_map_from_tgt.push_back(r);
			m_map_from_src[src_idx] = tgt_idx;
			//qDebug("  pxy-  src(%02i, %02i)     pxy(%02i, %02i)", first, last, src_first, src_last);
		}
	}

	if (src_first != -1)
	{
		//qDebug("  pxy-  beginInsertRows^^^(%02i, %02i)", src_first, src_last);
		beginInsertRows(QModelIndex(), src_first, src_last);
		endInsertRows();
	}
	return true;
}

bool SparseProxyModel::insertColumns (int first, int last, QModelIndex const & parent)
{
	int src_first = -1;
	int src_last = -1;
	for (int c = first; c < last + 1; ++c)
	{
		int const src_idx = m_cmap_from_src.size();
		int const tgt_idx = m_cmap_from_tgt.size();
		m_cmap_from_src.push_back(-1);
		if (filterAcceptsColumn(c, QModelIndex()))
		{
			if (src_first == -1)
				src_first = src_idx;
			src_last = src_idx;
			m_cmap_from_tgt.push_back(src_idx);
			m_cmap_from_src[src_idx] = tgt_idx;
			//qDebug("  pxy|  src(%02i, %02i)     pxy(%02i, %02i)", first, last, src_first, src_last);
		}
	}

	if (src_first != -1)
	{
		//qDebug("  pxy|  beginInsertCols^^^(%02i, %02i)", src_first, src_last);
		beginInsertColumns(QModelIndex(), src_first, src_last);
		endInsertColumns();
	}
	return true;
}

void SparseProxyModel::insertAllowedColumn (int src_col)
{
	if (src_col >= m_allowed_src_cols.size())
		m_allowed_src_cols.resize(src_col + 1);
	m_allowed_src_cols[src_col] = 1;
}

bool SparseProxyModel::filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const
{
	bool drop = true;
	for (int i = 0, ie = sourceModel()->rowCount(); i < ie; ++i)
	{
		QModelIndex const data_idx = sourceModel()->index(i, sourceColumn, QModelIndex());
		QVariant const & var = sourceModel()->data(data_idx);
		if (sourceColumn < m_allowed_src_cols.size() && m_allowed_src_cols[sourceColumn] == 0)
		{
			drop = false;
			break;
		}

		if (var.isValid() && !var.toString().isEmpty())
		{
			drop = false;
		}
	}
	return !drop;
}

bool SparseProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & /*sourceParent*/) const
{
	bool is_empty = true;
	for (int i = 0, ie = sourceModel()->columnCount(); i < ie; ++i)
	{
		QModelIndex const data_idx = sourceModel()->index(sourceRow, i, QModelIndex());
		QVariant const & var = sourceModel()->data(data_idx);
		if (var.isValid() && !var.toString().isEmpty())
			is_empty = false;
	}
	return !is_empty;
}

