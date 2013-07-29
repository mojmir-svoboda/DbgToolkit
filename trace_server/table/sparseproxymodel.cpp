#include "sparseproxymodel.h"
#include <connection.h>
#include <mainwindow.h>

SparseProxyModel::SparseProxyModel (QObject * parent)
	: BaseProxyModel(parent)
{ }

bool SparseProxyModel::filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const
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

