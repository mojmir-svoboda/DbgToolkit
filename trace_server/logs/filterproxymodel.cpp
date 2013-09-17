#include "filterproxymodel.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QListView>
#include <tlv_parser/tlv_encoder.h>
#include <trace_client/trace.h>
#include <connection.h>

FilterProxyModel::FilterProxyModel (QObject * parent, logs::LogWidget & lw)
	: BaseProxyModel(parent)
	, m_log_widget(lw)
	//, m_filter_state(lw.m_filter_state)
{ }

Qt::ItemFlags FilterProxyModel::flags (QModelIndex const & index) const
{
	return sourceModel()->flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool FilterProxyModel::filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const
{
	bool drop = true;
	for (int i = 0, ie = sourceModel()->rowCount(); i < ie; ++i)
	{
		QModelIndex const data_idx = sourceModel()->index(i, sourceColumn, QModelIndex());
		QVariant const & var = sourceModel()->data(data_idx);
		if (sourceColumn < m_allowed_src_cols.size() && m_allowed_src_cols[sourceColumn] == 1)
		{
			drop = false;
			break;
		}
	}
	return !drop;
}

bool FilterProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & /*sourceParent*/) const
{
	//if (inclusive_filters)
	//	return false;



	/*QModelIndex data_idx = sourceModel()->index(sourceRow, 0, QModelIndex());
	excluded |= m_filter_state.isBlockCollapsed(tid, data_idx.row());
	excluded |= data_idx.row() < m_log_widget.excludeContentToRow();*/
	return m_log_widget.filterMgr()->accept();
}









/* 
FilterProxyModel::FilterProxyModel (QObject * parent, logs::LogWidget & lw)
	: QAbstractProxyModel(parent)
	, m_log_widget(lw)
	, m_columns(0)
	, m_filter_state(lw.m_filter_state)
	, m_main_window(static_cast<Connection const *>(parent)->getMainWindow())
{ }

void FilterProxyModel::force_update ()
{
	emit layoutAboutToBeChanged();

	m_map_from_tgt.clear();
	QAbstractTableModel const * src_model = static_cast<QAbstractTableModel const *>(sourceModel());
	m_map_from_src.clear();
	m_map_from_src.resize(src_model->rowCount());
	m_columns = src_model->columnCount();
	for (int src_idx = 0, se = src_model->rowCount(); src_idx < se; ++src_idx)
	{
		if (filterAcceptsRow(src_idx, QModelIndex()))
		{
			int const tgt_idx = static_cast<int>(m_map_from_tgt.size());
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
	if (row < static_cast<int>(m_map_from_tgt.size())) // && column == reasonable
		return QAbstractItemModel::createIndex(row, column);
	return QModelIndex();
}
QModelIndex FilterProxyModel::parent (QModelIndex const & child) const
{
	return QModelIndex();
}

int FilterProxyModel::rowCount (QModelIndex const & parent) const
{
	return static_cast<int>(m_map_from_tgt.size());
}

int FilterProxyModel::columnCount (QModelIndex const & parent) const
{
	return m_columns;
}

QModelIndex FilterProxyModel::mapToSource (QModelIndex const & proxyIndex) const
{
	if (proxyIndex.isValid())
		if (proxyIndex.row() < static_cast<int>(m_map_from_tgt.size()))
			return QAbstractItemModel::createIndex(m_map_from_tgt[proxyIndex.row()], proxyIndex.column());
	return QModelIndex();
}

QModelIndex FilterProxyModel::mapFromSource (QModelIndex const & sourceIndex) const
{
	if (sourceIndex.isValid() && sourceIndex.row() < static_cast<int>(m_map_from_src.size()))
	{
		//qDebug("FPM: %s src.row=%i, src.sz=%u", __FUNCTION__, sourceIndex.row(), m_map_from_src.size());
		return QAbstractItemModel::createIndex(m_map_from_src[sourceIndex.row()], sourceIndex.column());
	}
	return QModelIndex();
}

bool FilterProxyModel::insertRows (int row, int count, QModelIndex const &parent)
{
	// @TODO: count == n

	int const src_idx = static_cast<int>(m_map_from_src.size());
	m_map_from_src.push_back(-1);

	if (filterAcceptsRow(src_idx, QModelIndex()))
	{
		emit layoutAboutToBeChanged();

		int const tgt_idx = static_cast<int>(m_map_from_tgt.size());
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

bool FilterProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & sourceParent) const
{
	//@TODO: toStdString is a performance hit too
	QString file, line;
	int const col_idx = m_log_widget.findColumn4Tag(tlv::tag_file);
	if (col_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, col_idx, QModelIndex());
		file = sourceModel()->data(data_idx).toString();
	}
	int const col_idx2 = m_log_widget.findColumn4Tag(tlv::tag_line);
	if (col_idx2 >= 0)
	{
		QModelIndex data_idx2 = sourceModel()->index(sourceRow, col_idx2, QModelIndex());
		line = sourceModel()->data(data_idx2).toString();
	}

	bool excluded = false;
	if (!file.isNull() && !line.isNull() && !file.isEmpty() && !line.isEmpty())
	{
		TreeModelItem ff;
		bool const ff_present = m_filter_state.isFileLinePresent(std::make_pair(file, line), ff);
		if (ff_present)
		{
			excluded |= ff.m_state == e_Unchecked;
		}
	}

	QString tid;
	int const tid_idx = m_log_widget.findColumn4Tag(tlv::tag_tid);
	if (tid_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, tid_idx, QModelIndex());
		tid = sourceModel()->data(data_idx).toString();
	}

	QString lvl;
	int const lvl_idx = m_log_widget.findColumn4Tag(tlv::tag_lvl);
	if (lvl_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, lvl_idx, QModelIndex());
		lvl = sourceModel()->data(data_idx).toString();
	}

	QString ctx;
	int const ctx_idx = m_log_widget.findColumn4Tag(tlv::tag_ctx);
	if (ctx_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, ctx_idx, QModelIndex());
		ctx = sourceModel()->data(data_idx).toString();
	}

	
	E_LevelMode lvlmode = e_LvlInclude;
	bool lvl_enabled = true;
	bool const lvl_present = m_filter_state.isLvlPresent(lvl, lvl_enabled, lvlmode);

	if (lvl_present)
	{
		if (lvl_enabled && lvlmode == e_LvlForceInclude)
			return true; // forced levels (errors etc)
		excluded |= !lvl_enabled;
	}

	bool inclusive_filters = false;
	for (int i = 0, ie = m_filter_state.m_filtered_regexps.size(); i < ie; ++i)
	{
		FilteredRegex const & fr = m_filter_state.m_filtered_regexps.at(i);
		if (!fr.m_is_enabled)
			continue;
		else
		{
			if (fr.m_state)
			{
				inclusive_filters = true;
				break;
			}
		}
	}
	if (m_filter_state.m_filtered_regexps.size() > 0)
	{
		QString msg;
		int const msg_idx = m_log_widget.findColumn4Tag(tlv::tag_msg);
		if (msg_idx >= 0)
		{
			QModelIndex data_idx = sourceModel()->index(sourceRow, msg_idx, QModelIndex());
			msg = sourceModel()->data(data_idx).toString();
		}

		for (int i = 0, ie = m_filter_state.m_filtered_regexps.size(); i < ie; ++i)
		{
			FilteredRegex const & fr = m_filter_state.m_filtered_regexps.at(i);
			if (fr.exactMatch(msg))
			{
				if (!fr.m_is_enabled)
					continue;
				else
				{
					if (fr.m_state)
						return true;
					else
						return false;
				}
			}
		}

	}

	for (int i = 0, ie = m_filter_state.m_filtered_strings.size(); i < ie; ++i)
	{
		FilteredString const & fr = m_filter_state.m_filtered_strings.at(i);
		if (!fr.m_is_enabled)
			continue;
		else
		{
			if (fr.m_state)
			{
				inclusive_filters = true;
				break;
			}
		}
	}
	if (m_filter_state.m_filtered_strings.size() > 0)
	{
		QString msg;
		int const msg_idx = m_log_widget.findColumn4Tag(tlv::tag_msg);
		if (msg_idx >= 0)
		{
			QModelIndex data_idx = sourceModel()->index(sourceRow, msg_idx, QModelIndex());
			msg = sourceModel()->data(data_idx).toString();
		}

		for (int i = 0, ie = m_filter_state.m_filtered_strings.size(); i < ie; ++i)
		{
			FilteredString const & fr = m_filter_state.m_filtered_strings.at(i);
			if (fr.match(msg))
			{
				if (!fr.m_is_enabled)
					continue;
				else
				{
					if (fr.m_state)
						return true;
					else
						return false;
				}
			}
		}

	}

	if (inclusive_filters)
		return false;

	excluded |= m_filter_state.isTIDExcluded(tid);

	bool ctx_enabled = true;
	bool const ctx_present = m_filter_state.isCtxPresent(ctx, ctx_enabled);
	if (ctx_present)
	{
		excluded |= !ctx_enabled;
	}

	QModelIndex data_idx = sourceModel()->index(sourceRow, 0, QModelIndex());
	excluded |= m_filter_state.isBlockCollapsed(tid, data_idx.row());
	excluded |= data_idx.row() < m_log_widget.excludeContentToRow();
	return !excluded;
}
*/
