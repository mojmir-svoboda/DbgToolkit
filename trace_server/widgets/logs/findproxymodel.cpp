#include "findproxymodel.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QListView>
#include "logwidget.h"
#include "logtablemodel.h"

namespace logs {

FindProxyModel::FindProxyModel (QObject * parent, FindConfig const & fc, FilterMgr const * f, LogTableModel * m, BaseProxyModel * pxy)
	: FilterProxyModel(parent, nullptr)
	, m_src_model(m)
	, m_src_proxy(pxy)
	, m_find_config(fc)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	m_accepted_rows_cache.reserve(1024);
}

FindProxyModel::~FindProxyModel ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

bool FindProxyModel::filterAcceptsIndex (QModelIndex const & sourceIndex, QModelIndex const & /*sourceParent*/)
{
	int sourceRow = sourceIndex.row();
	if (m_src_proxy)
		sourceRow = m_src_proxy->rowToSource(sourceRow);
	if (sourceRow >= 0 && sourceRow < m_src_model->rowCount())
	{
		for (int c = 0, ce = m_src_model->columnCount(); c < ce; ++c)
		{
			//if (!fr.m_where.m_states[c])
			if (c != proto::tag2col<proto::int_<proto::tag_msg>>::value)
				continue;

			QModelIndex const idx = m_src_model->index(sourceIndex.row(), c, QModelIndex());
			QVariant data = m_src_model->data(idx, Qt::DisplayRole);
			QString const & val = data.toString();
			FindConfig const & fc = m_find_config;
			if (fc.m_regexp)
			{
			}
			else
			{
				Qt::CaseSensitivity const cs = fc.m_case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
				if (val.contains(fc.m_str, cs))
				{
					return true;
				}
			}
		}
		return false; // no match
	}
	qWarning("no dcmd for source row, should not happen!");
	return false;
}

void FindProxyModel::commitBatchToModel (int src_from, int src_to)
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

}
