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
	QAbstractTableModel const * src_model = static_cast<QAbstractTableModel const *>(sourceModel());

	// rows
	m_map_from_tgt.clear();
	m_map_from_src.clear();
	m_map_from_src.reserve(src_model->rowCount());
	int tgt_idx = 0;
	for (size_t src_idx = 0, se = src_model->rowCount(); src_idx < se; ++src_idx)
	{
		if (filterAcceptsRow(src_idx, QModelIndex()))
		{
			m_map_from_src.insert(std::make_pair(src_idx, tgt_idx));
			++tgt_idx;
		}
	}

	m_map_from_tgt.resize(m_map_from_src.size());
	for (map_t::const_iterator it = m_map_from_src.begin(), ite = m_map_from_src.end(); it != ite; ++it)
		m_map_from_tgt[it->second] = it->first;

	// cols
	m_cmap_from_tgt.clear();
	m_cmap_from_src.clear();
	m_cmap_from_src.reserve(src_model->columnCount());
	int ctgt_idx = 0;
	for (size_t src_idx = 0, se = src_model->columnCount(); src_idx < se; ++src_idx)
	{
		if (filterAcceptsColumn(src_idx, QModelIndex()))
		{
			m_cmap_from_src.insert(std::make_pair(src_idx, ctgt_idx));
			++ctgt_idx;
		}
	}

	m_cmap_from_tgt.resize(m_cmap_from_src.size());
	for (map_t::const_iterator it = m_cmap_from_src.begin(), ite = m_cmap_from_src.end(); it != ite; ++it)
		m_cmap_from_tgt[it->second] = it->first;

	emit layoutChanged();
}

QVariant SparseProxyModel::data (QModelIndex const & index, int role) const
{
	QModelIndex const src_idx = mapToSource(index);
	return sourceModel()->data(src_idx, role);
}

void SparseProxyModel::insertCol (int c)
{
	int pos0 = 0;
	int pos1 = m_cmap_from_src.size();
	map_t::iterator it0 = m_cmap_from_src.lower_bound(c);
	map_t::iterator it1 = m_cmap_from_src.upper_bound(c);

	if (it0 != m_cmap_from_src.end() && it1 != m_cmap_from_src.end() && it0->first == c)
		return;

	if (it0 != m_cmap_from_src.end())
		pos0 = it0->second;
	if (it1 != m_cmap_from_src.end())
	{
		pos1 = it1->second;
		for (map_t::iterator iit = it1, iite = m_cmap_from_src.end(); iit != iite; ++iit)
		{
			iit->second += 1;
		}
	}
	//qDebug("  pxy|  beginInsertCols^^^(%02i, %02i)", pos0, pos1);
	beginInsertColumns(QModelIndex(), pos0, pos0);
	m_cmap_from_src.insert(std::make_pair(c, pos1));

	m_cmap_from_tgt.resize(m_cmap_from_src.size()); // ugh
	for (map_t::const_iterator it = m_cmap_from_src.begin(), ite = m_cmap_from_src.end(); it != ite; ++it)
		m_cmap_from_tgt[it->second] = it->first;
	endInsertColumns();
}

void SparseProxyModel::insertRow (int c)
{
	int pos0 = 0;
	int pos1 = m_map_from_src.size();
	map_t::iterator it0 = m_map_from_src.lower_bound(c);
	map_t::iterator it1 = m_map_from_src.upper_bound(c);

	if (it0 != m_map_from_src.end() && (it1 != m_map_from_src.end() && it0->first == c))
		return;

	if (it0 != m_map_from_src.end())
		pos0 = it0->second;
	if (it1 != m_map_from_src.end())
	{
		pos1 = it1->second;
		for (map_t::iterator iit = it1, iite = m_map_from_src.end(); iit != iite; ++iit)
		{
			iit->second += 1;
		}
	}

	beginInsertRows(QModelIndex(), pos0, pos0);
	m_map_from_src.insert(std::make_pair(c, pos1));

	m_map_from_tgt.resize(m_map_from_src.size()); // ugh
	for (map_t::const_iterator it = m_map_from_src.begin(), ite = m_map_from_src.end(); it != ite; ++it)
		m_map_from_tgt[it->second] = it->first;
	endInsertRows();
}


bool SparseProxyModel::setData (QModelIndex const & src_index, QVariant const & value, int role)
{
	if (filterAcceptsColumn(src_index.column(), QModelIndex()))
		insertCol(src_index.column());

	if (filterAcceptsRow(src_index.row(), QModelIndex()))
		insertRow(src_index.row());

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
	map_t::const_iterator it = m_map_from_src.find(row);
	if (it != m_map_from_src.end())
		return true;
	return false;
}

bool SparseProxyModel::colInProxy (int col) const
{
	return colToSource(col) > -1;
}

int SparseProxyModel::colToSource (int col) const
{
	int val = -1;
	if (col < static_cast<int>(m_cmap_from_tgt.size()))
		return m_cmap_from_tgt[col];
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
	if (sourceIndex.isValid() && sourceIndex.row() < static_cast<int>(m_map_from_src.size()))
	{
		map_t::const_iterator rit = m_map_from_src.find(sourceIndex.row());
		map_t::const_iterator cit = m_cmap_from_src.find(sourceIndex.column());
		if (cit != m_cmap_from_src.end() && rit != m_map_from_src.end())
			return QAbstractItemModel::createIndex(rit->second, cit->second);
	}
	return QModelIndex();
}

QModelIndex SparseProxyModel::mapNearestFromSource (QModelIndex const & sourceIndex) const
{
	if (sourceIndex.isValid() && sourceIndex.row() < static_cast<int>(m_map_from_src.size())
			&& sourceIndex.column() < static_cast<int>(m_cmap_from_src.size()))
	{
		/*if (m_cmap_from_src[sourceIndex.column()] == -1) // column is not in filtered view, finding nearest
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
		return QAbstractItemModel::createIndex(m_map_from_src[sourceIndex.row()], m_cmap_from_src[sourceIndex.column()]);*/
	}
	return QModelIndex();
}


bool SparseProxyModel::insertRows (int first, int last, QModelIndex const & parent)
{
	return true;
}

bool SparseProxyModel::insertColumns (int first, int last, QModelIndex const & parent)
{
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

