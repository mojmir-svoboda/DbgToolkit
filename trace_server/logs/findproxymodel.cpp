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

bool FindProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & /*sourceParent*/) const
{
	if (sourceRow < m_log_widget.m_src_model->dcmds().size())
	{
		DecodedCommand const & dcmd = m_log_widget.m_src_model->dcmds()[sourceRow];
		QString line;
		/*if (dcmd.getString(tlv::tag_line, line) && line == "12")
			printf("ee");*/
		bool const accepted = m_log_widget.filterMgr()->accept(dcmd);
		return accepted;
	}
	qWarning("no dcmd for source row, should not happen!");
	return true;
}
