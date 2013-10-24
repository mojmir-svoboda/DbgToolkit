#include "filterproxymodel.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QListView>
#include <tlv_parser/tlv_encoder.h>
#include <trace_client/trace.h>
#include <connection.h>
#include "logtablemodel.h"

FilterProxyModel::FilterProxyModel (QObject * parent, logs::LogWidget & lw)
	: BaseProxyModel(parent)
	, m_log_widget(lw)
	, m_column_count(0)
	//, m_filter_state(lw.m_filter_state)
{ }

void FilterProxyModel::resizeToCfg ()
{
	if (m_log_widget.m_config.m_columns_setup.size() > 0 && m_column_count == 0)
	{
		int const last = m_log_widget.m_config.m_columns_setup.size() - 1;
		beginInsertColumns(QModelIndex(), m_column_count, last);
		insertColumns(m_column_count, last);
		m_column_count = last + 1;
		endInsertColumns();
	}
}


Qt::ItemFlags FilterProxyModel::flags (QModelIndex const & index) const
{
	return sourceModel()->flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool FilterProxyModel::filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const
{
	return true;
}

bool FilterProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & /*sourceParent*/) const
{
	if (sourceRow < m_log_widget.m_src_model->dcmds().size())
	{
		DecodedCommand const & dcmd = m_log_widget.m_src_model->dcmds()[sourceRow];
		bool const accepted = m_log_widget.filterMgr()->accept(dcmd);
		return accepted;
	}
	qWarning("no dcmd for source row, should not happen!");
	return true;
}



void FilterProxyModel::commitBatchToModel (int src_from, int src_to, BatchCmd const & batch)
{
	int const rows = src_to - src_from;
	int const from = m_map_from_tgt.size();

	m_map_from_src.reserve(m_log_widget.m_src_model->rowCount());

	QVector<int> accepted_rows; // grr
	accepted_rows.reserve(32);
	int tgt_idx = m_map_from_tgt.size();
	for (size_t src_idx = src_from; src_idx < src_to; ++src_idx)
	{
		if (filterAcceptsRow(src_idx, QModelIndex()))
		{
			accepted_rows.push_back(src_idx);
			m_map_from_src.insert(std::make_pair(src_idx, tgt_idx));
			++tgt_idx;
		}
	}

	int const n_accepted = accepted_rows.size();
	m_map_from_tgt.reserve(from + n_accepted);
	int const to = from + n_accepted;
	beginInsertRows(QModelIndex(), from, to);
	for (int i = 0, ie = n_accepted; i < ie; ++i)
	{
		m_map_from_tgt.push_back(accepted_rows[i]);
	}
	endInsertRows();

	//@FIXME l8r: this does not work in general!
	if (m_cmap_from_src.size() < m_log_widget.m_src_model->columnCount())
	{
		int const from = m_cmap_from_src.size();
		m_cmap_from_tgt.clear();
		m_cmap_from_src.clear();
		m_cmap_from_src.reserve(m_log_widget.m_src_model->columnCount());
		int ctgt_idx = 0;
		for (size_t src_idx = 0, se = m_log_widget.m_src_model->columnCount(); src_idx < se; ++src_idx)
		{
			if (filterAcceptsColumn(src_idx, QModelIndex()))
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



