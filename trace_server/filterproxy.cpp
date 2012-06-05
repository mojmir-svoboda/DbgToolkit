#include "filterproxy.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QListView>
#include <tlv_parser/tlv_encoder.h>
#include <trace_client/trace.h>

void FilterProxyModel::force_update ()
{
	emit layoutAboutToBeChanged();

	m_map_from_tgt.clear();
	for (size_t src_idx = 0, se = m_map_from_src.size(); src_idx < se; ++src_idx)
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

QVariant FilterProxyModel::data (QModelIndex const & index, int role) const
{
	QModelIndex const src_idx = mapToSource(index);
	return sourceModel()->data(src_idx, role);
}

Qt::ItemFlags FilterProxyModel::flags (QModelIndex const & index) const
{
	return sourceModel()->flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex FilterProxyModel::index (int row, int column, QModelIndex const & parent) const
{
	if (row < m_map_from_tgt.size()) // && column == reasonable
		return QAbstractItemModel::createIndex(row, column, 0);
	return QModelIndex();
}
QModelIndex FilterProxyModel::parent (QModelIndex const & child) const
{
	return QModelIndex();
}

int FilterProxyModel::rowCount (QModelIndex const & parent) const
{
	return m_map_from_tgt.size();
}

int FilterProxyModel::columnCount (QModelIndex const & parent) const
{
	return m_columns;
}

QModelIndex FilterProxyModel::mapToSource (QModelIndex const & proxyIndex) const
{
	if (proxyIndex.isValid())
		if (proxyIndex.row() < m_map_from_tgt.size())
			return QAbstractItemModel::createIndex(m_map_from_tgt[proxyIndex.row()], proxyIndex.column(), 0);
	return QModelIndex();
}

QModelIndex FilterProxyModel::mapFromSource (QModelIndex const & sourceIndex) const
{
	if (sourceIndex.isValid() && sourceIndex.row() < m_map_from_src.size())
	{
		//qDebug("FPM: %s src.row=%i, src.sz=%u", __FUNCTION__, sourceIndex.row(), m_map_from_src.size());
		return QAbstractItemModel::createIndex(m_map_from_src[sourceIndex.row()], sourceIndex.column(), 0);
	}
	return QModelIndex();
}

bool FilterProxyModel::insertRows (int row, int count, QModelIndex const &parent)
{
	// @TODO: count == n

	int const src_idx = m_map_from_src.size();
	m_map_from_src.push_back(-1);

	if (filterAcceptsRow(src_idx, QModelIndex()))
	{
		emit layoutAboutToBeChanged();

		int const tgt_idx = m_map_from_tgt.size();
		//beginInsertRows(parent, tgt_idx, tgt_idx);

		m_map_from_tgt.push_back(src_idx);
		m_map_from_src[src_idx] = tgt_idx;
		
		//endInsertRows();	//@TODO: this causes performance hit too

		emit layoutChanged();
	}
	return true;
}

bool FilterProxyModel::insertColumns (int column, int count, QModelIndex const & parent)
{
	if (column >= m_columns)
		++m_columns;
	return true;
}

bool FilterProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & /*sourceParent*/) const
{
	//@TODO: toStdString is a performance hit too
	QString file, line;
	int const col_idx = m_session_state.findColumn4Tag(tlv::tag_file);
	if (col_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, col_idx, QModelIndex());
		file = sourceModel()->data(data_idx).toString();
	}
	int const col_idx2 = m_session_state.findColumn4Tag(tlv::tag_line);
	if (col_idx2 >= 0)
	{
		QModelIndex data_idx2 = sourceModel()->index(sourceRow, col_idx2, QModelIndex());
		line = sourceModel()->data(data_idx2).toString();
	}

	bool excluded = false;
	excluded |= m_session_state.isFileLineExcluded(std::make_pair(file.toStdString(), line.toStdString()));

	QString tid;
	int const tid_idx = m_session_state.findColumn4Tag(tlv::tag_tid);
	if (tid_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, tid_idx, QModelIndex());
		tid = sourceModel()->data(data_idx).toString();
	}

	QString lvl;
	int const lvl_idx = m_session_state.findColumn4Tag(tlv::tag_lvl);
	if (lvl_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, lvl_idx, QModelIndex());
		lvl = sourceModel()->data(data_idx).toString();
	}

	QString ctx;
	int const ctx_idx = m_session_state.findColumn4Tag(tlv::tag_ctx);
	if (ctx_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, ctx_idx, QModelIndex());
		ctx = sourceModel()->data(data_idx).toString();
	}

	bool inclusive_filters = false;
	for (int i = 0, ie = m_session_state.m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex const & fr = m_session_state.m_filtered_regexps.at(i);
		if (!fr.m_is_enabled)
			continue;
		else
		{
			if (fr.m_is_inclusive)
			{
				inclusive_filters = true;
				break;
			}
		}
	}

	if (m_session_state.m_filtered_regexps.size() > 0)
	{
		QString msg;
		int const msg_idx = m_session_state.findColumn4Tag(tlv::tag_msg);
		if (msg_idx >= 0)
		{
			QModelIndex data_idx = sourceModel()->index(sourceRow, msg_idx, QModelIndex());
			msg = sourceModel()->data(data_idx).toString();
		}

		for (int i = 0, ie = m_session_state.m_filtered_regexps.size(); i < ie; ++i)
		{
			FilteredRegex const & fr = m_session_state.m_filtered_regexps.at(i);
			if (fr.exactMatch(msg))
			{
				if (!fr.m_is_enabled)
					continue;
				else
				{
					if (fr.m_is_inclusive)
						return true;
					else
						return false;
				}
			}
		}

	}

	if (inclusive_filters)
		return false;

	excluded |= m_session_state.isTIDExcluded(tid.toStdString());
	excluded |= m_session_state.isLvlExcluded(lvl.toStdString());
	excluded |= m_session_state.isCtxExcluded(ctx.toULongLong());

	QModelIndex data_idx = sourceModel()->index(sourceRow, 0, QModelIndex());
	excluded |= m_session_state.isBlockCollapsed(tid, data_idx.row());
	excluded |= data_idx.row() < m_session_state.excludeContentToRow();
	return !excluded;
}

