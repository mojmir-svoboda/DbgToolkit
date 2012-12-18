#include "sparseproxy.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QListView>
#include "connection.h"
#include "mainwindow.h"

SparseProxyModel::SparseProxyModel (QObject * parent)
	: QAbstractProxyModel(parent)
	, m_columns(0)
	, m_main_window(static_cast<Connection const *>(parent)->getMainWindow())
{ }

void SparseProxyModel::force_update ()
{
	emit layoutAboutToBeChanged();

	m_map_from_tgt.clear();
	QAbstractTableModel const * src_model = static_cast<QAbstractTableModel const *>(sourceModel());
	m_map_from_src.clear();
	m_map_from_src.resize(src_model->rowCount());
	m_columns = src_model->columnCount();
	for (size_t src_idx = 0, se = src_model->rowCount(); src_idx < se; ++src_idx)
	{
		if (filterAcceptsRow(src_idx, QModelIndex()))
		{
			int const tgt_idx = m_map_from_tgt.size();
			m_map_from_tgt.push_back(src_idx);
			m_map_from_src[src_idx] = tgt_idx;
		}
	}
	emit layoutChanged();
}

QVariant SparseProxyModel::data (QModelIndex const & index, int role) const
{
	QModelIndex const src_idx = mapToSource(index);
	return sourceModel()->data(src_idx, role);
}

Qt::ItemFlags SparseProxyModel::flags (QModelIndex const & index) const
{
	return sourceModel()->flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex SparseProxyModel::index (int row, int column, QModelIndex const & parent) const
{
	if (row < static_cast<int>(m_map_from_tgt.size())) // && column == reasonable
		return QAbstractItemModel::createIndex(row, column, 0);
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
	return m_columns;
}

QModelIndex SparseProxyModel::mapToSource (QModelIndex const & proxyIndex) const
{
	if (proxyIndex.isValid())
		if (proxyIndex.row() < static_cast<int>(m_map_from_tgt.size()))
			return QAbstractItemModel::createIndex(m_map_from_tgt[proxyIndex.row()], proxyIndex.column(), 0);
	return QModelIndex();
}

QModelIndex SparseProxyModel::mapFromSource (QModelIndex const & sourceIndex) const
{
	if (sourceIndex.isValid() && sourceIndex.row() < static_cast<int>(m_map_from_src.size()))
	{
		//qDebug("FPM: %s src.row=%i, src.sz=%u", __FUNCTION__, sourceIndex.row(), m_map_from_src.size());
		return QAbstractItemModel::createIndex(m_map_from_src[sourceIndex.row()], sourceIndex.column(), 0);
	}
	return QModelIndex();
}

bool SparseProxyModel::insertRows (int row, int count, QModelIndex const &parent)
{
	int const src_idx = m_map_from_src.size();
	m_map_from_src.push_back(-1);

	if (filterAcceptsRow(src_idx, QModelIndex()))
	{
		emit layoutAboutToBeChanged();

		int const tgt_idx = m_map_from_tgt.size();

		m_map_from_tgt.push_back(src_idx);
		m_map_from_src[src_idx] = tgt_idx;
		
		emit layoutChanged();
	}
	return true;
}

bool SparseProxyModel::insertColumns (int column, int count, QModelIndex const & parent)
{
	if (column >= m_columns)
	{
		emit layoutAboutToBeChanged();
		++m_columns;
		emit layoutChanged();
	}
	return true;
}

bool SparseProxyModel::filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const
{
	bool is_empty = true;
	for (int i = 0, ie = sourceModel()->rowCount(); i < ie; ++i)
	{
		QModelIndex const data_idx = sourceModel()->index(i, sourceColumn, QModelIndex());
		QVariant const & var = sourceModel()->data(data_idx);
		if (var.isValid() && !var.toString().isEmpty())
			is_empty = false;
	}
	return !is_empty;
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

