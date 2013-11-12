#include "findproxymodel.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QListView>
//#include <tlv_parser/tlv_encoder.h>
//#include <trace_client/trace.h>
//#include <connection.h>
#include "logwidget.h"
#include "logtablemodel.h"

FindProxyModel::FindProxyModel (QObject * parent, logs::LogWidget & lw)
	: BaseProxyModel(parent)
	, m_log_widget(lw)
	, m_column_count(0)
{ }

FindProxyModel::~FindProxyModel ()
{
	qDebug("%s", __FUNCTION__);
}

void FindProxyModel::resizeToCfg ()
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


Qt::ItemFlags FindProxyModel::flags (QModelIndex const & index) const
{
	return sourceModel()->flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool FindProxyModel::filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const
{
	return true;
}

QModelIndex FindProxyModel::sibling (int row, int column, QModelIndex const & idx) const
{
	return (row == idx.row() && column == idx.column()) ? idx : index(row, column, parent(idx));
}

bool FindProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & /*sourceParent*/) const
{
	if (sourceRow < m_log_widget.m_src_model->dcmds().size())
	{
		DecodedCommand const & dcmd = m_log_widget.m_src_model->dcmds()[sourceRow];

		for (size_t i = 0, ie = dcmd.m_tvs.size(); i < ie; ++i)
		{
			QString const & val = dcmd.m_tvs[i].m_val;
			FindConfig const & fc = m_log_widget.m_config.m_find_config;
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
