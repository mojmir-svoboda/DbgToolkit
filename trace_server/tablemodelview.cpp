#include "tablemodelview.h"
#include <QBrush>
#include <QColor>
#include <QAbstractProxyModel>
#include <trace_client/trace.h>

TableModelView::TableModelView (QObject * parent, QVector<QString> & hhdr, QVector<int> & hsize)
	: QAbstractTableModel(parent)
	, m_columnCount(0)
	, m_hhdr(hhdr)
	, m_hsize(hsize)
{
	size_t const prealloc_size = 128;
	m_rows.reserve(prealloc_size);
}

TableModelView::~TableModelView ()
{
	qDebug("%s", __FUNCTION__);
}

int TableModelView::rowCount (const QModelIndex & /*parent*/) const { return m_rows.size(); }

int TableModelView::columnCount (const QModelIndex & /*parent*/) const
{
	return m_columnCount;
}

inline bool TableModelView::checkExistence (QModelIndex const & index) const
{
	return index.row() < (int)m_rows.size() && index.column() < (int)m_rows[index.row()].size();
}

QVariant TableModelView::data (const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
	{
		QString str("");
		if (checkExistence(index))
		{
			return m_rows[index.row()][index.column()];
		}
		return str;
	}

	return QVariant();
}

bool TableModelView::setData (QModelIndex const & index, QVariant const & value, int role)
{
	if (!index.isValid()) return false;
	if (role == Qt::DisplayRole)
		return false;
	else if (role == Qt::EditRole)
	{
		if (checkExistence(index))
		{
			int const x = index.column();
			int const y = index.row();
			m_rows.at(y).at(x) = value.toString();
			emit dataChanged(index, index);
		}
	}

	return true;
}


QVariant TableModelView::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		if (section + 1 > m_hhdr.size())
		{
			m_hhdr.resize(section + 1);
		}
		return m_hhdr.at(section);
	}
	return QVariant();
}

bool  TableModelView::setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role)
{
	if (role == Qt::EditRole && orientation == Qt::Horizontal)
	{
		if (section + 1 > m_hhdr.size())
		{
			m_hhdr.resize(section);
		}
		m_hhdr[section] = value.toString();
	}
	return true;
}

void TableModelView::transactionStart (size_t n)
{
	int const row = rowCount();
	beginInsertRows(QModelIndex(), row, row + n);
}

void TableModelView::transactionCommit ()
{
	endInsertRows();
}

void TableModelView::emitLayoutChanged ()
{
	emit layoutChanged();
}

void TableModelView::appendTableXY (int x, int y, QString const & cmd)
{
	QStringList const values = cmd.split("|");
	int const n_cols = values.size();

	if (y < 0)
	{
		transactionStart(0);
		m_rows.push_back(columns_t());
		transactionCommit();
		if (x < 0)
		{
			m_rows.back().resize(n_cols);
			x = 0;
		}
		else
		{
			m_rows.back().resize(x + n_cols);

		}
		y = m_rows.size() - 1;
	}
	else
	{
		if (y >= m_rows.size())
		{
			transactionStart(y);
			m_rows.resize(y + 1);
			transactionCommit();
		}

		if (x < 0)
		{
			beginInsertColumns(QModelIndex(), m_columnCount, m_columnCount + n_cols);
			++m_columnCount;
			size_t const curr_sz = m_rows[y].size();
			m_rows[y].resize(curr_sz + 1);
			x = curr_sz;
			endInsertColumns();
			// @TODO: updatnout size kontejneru s predchozim poctem elementu
		}
		else
		{
			if (x + n_cols >= m_rows.back().size())
			{
				m_rows.back().resize(x + n_cols);
			}
		}
	}

	if (m_columnCount < n_cols)
	{
		beginInsertColumns(QModelIndex(), m_columnCount, n_cols - m_columnCount);
		insertColumns(m_columnCount, n_cols - m_columnCount);
		m_columnCount = n_cols;
		endInsertColumns();
		// @TODO: updatnout size kontejneru s predchozim poctem elementu
	}

	for (int ix = x, ixe = x + n_cols; ix < ixe; ++ix)
	{
		QModelIndex const idx = index(y, ix, QModelIndex());
		setData(idx, values.at(ix - x), Qt::EditRole);
	}
}

