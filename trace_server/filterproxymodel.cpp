#include "filterproxymodel.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QListView>
#include <tlv_parser/tlv_encoder.h>
#include <trace_client/trace.h>
#include <connection.h>
#include "basetablemodel.h"

FilterProxyModel::FilterProxyModel (QObject * parent, FilterMgr const * f, BaseTableModel * m)
	: BaseProxyModel(parent)
	, m_dcmds_model(m)
	, m_column_count(0)
	, m_filter_mgr(f)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

FilterProxyModel::~FilterProxyModel ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

void FilterProxyModel::resizeToCfg (logs::LogConfig const & config)
{
  //@TODO: dedup: logtablemodel, findproxy, filterproxy
	if (config.m_columns_setup.size() > m_column_count)
	{
		int const last = static_cast<int>(config.m_columns_setup.size()) - 1;
		beginInsertColumns(QModelIndex(), m_column_count, last);
		insertColumns(m_column_count, last);
		m_column_count = last + 1;
		endInsertColumns();
	}
  else if (config.m_columns_setup.size() < m_column_count)
	{
		int const last = static_cast<int>(config.m_columns_setup.size()) + 1;
		beginRemoveColumns(QModelIndex(), last, m_column_count);
		removeColumns(last, m_column_count);
		m_column_count = last - 1;
		endInsertColumns();
	}
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

bool FilterProxyModel::filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const
{
	return true;
}

bool FilterProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & /*sourceParent*/) const
{
	if (sourceRow < m_dcmds_model->dcmds().size())
	{
		DecodedCommand const & dcmd = m_dcmds_model->dcmds()[sourceRow];
		bool const accepted = m_filter_mgr->accept(dcmd);
		return accepted;
	}
	qWarning("no dcmd for source row, should not happen!");
	return true;
}

void FilterProxyModel::clearModel ()
{
	BaseProxyModel::clearModel();
	m_column_count = 0;
}

void FilterProxyModel::clearModelData ()
{
	BaseProxyModel::clearModelData();
}


void FilterProxyModel::commitBatchToModel (int src_from, int src_to, BatchCmd const & batch)
{
	int const rows = batch.m_dcmds.size();
	int const from = static_cast<int>(m_map_from_tgt.size());

	m_map_from_src.reserve(m_dcmds_model->rowCount());

	std::vector<int> accepted_rows; // grr @FIXME
	accepted_rows.reserve(32);
	size_t tgt_idx = m_map_from_tgt.size();
	for (size_t src_idx = src_from; src_idx < src_to; ++src_idx)
	{
		if (filterAcceptsRow(static_cast<int>(src_idx), QModelIndex()))
		{
			accepted_rows.push_back(static_cast<int>(src_idx));
			m_map_from_src.insert(std::make_pair(src_idx, tgt_idx));
			++tgt_idx;
		}
	}

	if (int const n_accepted = static_cast<int>(accepted_rows.size()))
	{
		m_map_from_tgt.reserve(from + n_accepted);
		int const to = from + n_accepted - 1;
		beginInsertRows(QModelIndex(), from, to);
		for (int i = 0, ie = n_accepted; i < ie; ++i)
		{
			m_map_from_tgt.push_back(accepted_rows[i]);
		}
		endInsertRows();
	}

	//@FIXME l8r: this does not work in general!
	if (m_cmap_from_src.size() < m_dcmds_model->columnCount())
	{
		int const from = static_cast<int>(m_cmap_from_src.size());
		m_cmap_from_tgt.clear();
		m_cmap_from_src.clear();
		m_cmap_from_src.reserve(m_dcmds_model->columnCount());
		int ctgt_idx = 0;
		for (size_t src_idx = 0, se = m_dcmds_model->columnCount(); src_idx < se; ++src_idx)
		{
			if (filterAcceptsColumn(static_cast<int>(src_idx), QModelIndex()))
			{
				m_cmap_from_src.insert(std::make_pair(src_idx, ctgt_idx));
				++ctgt_idx;
			}
		}
		int const to = ctgt_idx - 1;
		beginInsertColumns(QModelIndex(), from, to);
		m_cmap_from_tgt.resize(m_cmap_from_src.size());
		for (map_t::const_iterator it = m_cmap_from_src.begin(), ite = m_cmap_from_src.end(); it != ite; ++it)
		{
			m_cmap_from_tgt[it->second] = it->first;
		}
		endInsertColumns();
	}
}


ExtFilterProxyModel::ExtFilterProxyModel (QObject * parent, FilterMgr const * f, BaseTableModel * m)
	: FilterProxyModel(parent, f, m)
  , m_scopes_enabled(true)
  , m_dt_scopes_enabled(true)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

bool ExtFilterProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & sourceParent) const
{
	if (sourceRow < m_dcmds_model->dcmds().size())
	{
		DecodedCommand const & dcmd = m_dcmds_model->dcmds()[sourceRow];

    bool const is_scope = (dcmd.m_hdr.cmd == tlv::cmd_scope_entry || dcmd.m_hdr.cmd == tlv::cmd_scope_exit);
    if (is_scope && !m_scopes_enabled)
			return false;
	}
  return FilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

