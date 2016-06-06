#include "logfilterproxymodel.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QListView>
#include <connection.h>
#include "logtablemodel.h"

namespace logs {

FilterProxyModel::FilterProxyModel (QObject * parent, FilterMgr * f)
	: BaseProxyModel(parent)
	, m_filter_mgr(f)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

FilterProxyModel::~FilterProxyModel ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

void FilterProxyModel::setSourceModel (QAbstractItemModel *sourceModel)
{
	qDebug("%s this=0x%08x src_model=0x%08x", __FUNCTION__, this, sourceModel);
	BaseProxyModel::setSourceModel(sourceModel);
}

Qt::ItemFlags FilterProxyModel::flags (QModelIndex const & index) const
{
	QAbstractItemModel * src_model = sourceModel();
	return src_model->flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex FilterProxyModel::sibling (int row, int column, QModelIndex const & idx) const
{
	return (row == idx.row() && column == idx.column()) ? idx : index(row, column, parent(idx));
}

bool FilterProxyModel::filterAcceptsIndex (QModelIndex const & sourceIndex, QModelIndex const & /*sourceParent*/)
{
	//if (sourceModel()->check_index(sourceIndex))
	{
		bool const accepted = m_filter_mgr->accept(sourceIndex);
		return accepted;
	}
	qWarning("invalid sourceRow for proxy!");
	return true;
}

void FilterProxyModel::clearModel ()
{
	BaseProxyModel::clearModel();
}

void FilterProxyModel::clearModelData ()
{
	BaseProxyModel::clearModelData();
}


LogFilterProxyModel::LogFilterProxyModel (QObject * parent, FilterMgr * f, LogTableModel * m)
	: FilterProxyModel(parent, f)
	, m_src_model(m)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	m_accepted_rows_cache.reserve(1024);
}

LogFilterProxyModel::~LogFilterProxyModel ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

void LogFilterProxyModel::commitBatchToModel (int src_from, int src_to)
{
	int const rows = src_to - src_from;
	int const from = static_cast<int>(m_map_from_tgt.size());

	m_map_from_src.reserve(m_src_model->rowCount());
	m_accepted_rows_cache.clear();

	size_t tgt_idx = m_map_from_tgt.size();
	for (int src_row = src_from; src_row < src_to; ++src_row)
	{
		QModelIndex const src_idx = m_src_model->index(src_row, 0, QModelIndex());
		if (filterAcceptsIndex(src_idx, QModelIndex()))
		{
			m_accepted_rows_cache.push_back(static_cast<int>(src_row));
			m_map_from_src.insert(std::make_pair(src_row, tgt_idx));
			++tgt_idx;
		}
	}

	if (int const n_accepted = static_cast<int>(m_accepted_rows_cache.size()))
	{
		m_map_from_tgt.reserve(from + n_accepted);
		int const to = from + n_accepted - 1;
		beginInsertRows(QModelIndex(), from, to);
		for (int i = 0, ie = n_accepted; i < ie; ++i)
		{
			m_map_from_tgt.push_back(m_accepted_rows_cache[i]);
		}
		endInsertRows();
	}
}

QVariant LogFilterProxyModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	return m_src_model->headerData(section, orientation, role);
}

ExtLogFilterProxyModel::ExtLogFilterProxyModel (QObject * parent, FilterMgr * f, LogTableModel * m)
	: LogFilterProxyModel(parent, f, m)
	, m_scopes_enabled(true)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

bool ExtLogFilterProxyModel::filterAcceptsIndex (QModelIndex const & sourceIndex, QModelIndex const & sourceParent)
{
	const proto::rowdata_t rowdata = m_src_model->getRecordDataForRow(sourceIndex.row());

	QVariant const col_flags_qv = m_src_model->rawData().getRecordData(std::get<1>(rowdata)->m_flags, sourceIndex.column());
	proto::Flags col_flags = col_flags_qv.value<proto::Flags>();

	bool const is_scope = (col_flags.m_scope_type != LogScopeType_scopeNone);
	if (is_scope && !m_scopes_enabled)
		return false;
	return LogFilterProxyModel::filterAcceptsIndex(sourceIndex, sourceParent);
}

} // namespace logs
