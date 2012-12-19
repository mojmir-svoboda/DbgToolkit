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
	, m_proxy(0)
{
	size_t const prealloc_size = 128;
	m_rows.reserve(prealloc_size);
}

TableModelView::~TableModelView ()
{
	qDebug("%s", __FUNCTION__);
}

int TableModelView::rowCount (QModelIndex const & /*parent*/) const { return m_rows.size(); }

int TableModelView::columnCount (QModelIndex const & /*parent*/) const
{
	return m_columnCount;
}

inline bool TableModelView::checkExistence (QModelIndex const & index) const
{
	return index.row() < (int)m_rows.size() && index.column() < (int)m_rows[index.row()].size();
}

QVariant TableModelView::data (QModelIndex const & index, int role) const
{
	if (!index.isValid())
		return QVariant();

	QVariant value;
	if (checkExistence(index))
	{
		value = m_rows[index.row()][index.column()];
	}

	if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
	{
		return value;
	}
	else if (role == Qt::BackgroundRole)
	{
		if (value.toString().isEmpty())
		{
			return Qt::gray;
		}
	}
	return QVariant();
}

bool TableModelView::setData (QModelIndex const & index, QVariant const & value, int role)
{
	if (!index.isValid()) return false;
	if (role == Qt::DisplayRole) return false;
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
		if (section == -1)
			return QVariant();
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
		if (section == -1)
			return false;
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

	//qDebug("append: x=%i y=%i cols=%i val=%s", x, y, n_cols, cmd.toAscii());

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
			//qDebug("  append: y>rows.sz resize %i -> %i", y, y);
			beginInsertRows(QModelIndex(), m_rows.size(), y);
			m_rows.resize(y + 1);
			transactionCommit();

			if (m_proxy)
				m_proxy->insertRows(m_rows.size(), y);
		}

		if (x < 0)
		{
			beginInsertColumns(QModelIndex(), m_columnCount, m_columnCount + n_cols - 1);
			++m_columnCount;
			size_t const curr_sz = m_rows[y].size();
			m_rows[y].resize(curr_sz + 1);
			x = curr_sz;
			endInsertColumns();
			// @TODO: updatnout size kontejneru s predchozim poctem elementu
		}
		else
		{
			//qDebug("  append: x>=0 ", m_rows.back().size(), x + n_cols);
			if (x + n_cols >= m_rows.back().size())
			{
				//qDebug("  append: x>=0  resize %i -> %i", m_rows.back().size(), x + n_cols);
				m_rows.back().resize(x + n_cols);
			}
		}
	}

	//qDebug("  m_columnCount < n_cols  %i < %i", m_columnCount, x + n_cols);
	if (m_columnCount < x + n_cols)
	{
		beginInsertColumns(QModelIndex(), m_columnCount, x + n_cols - 1);
		insertColumns(m_columnCount, x + n_cols - 1);
		m_columnCount = x + n_cols;
		endInsertColumns();

		if (m_proxy)
			m_proxy->insertColumns(m_columnCount, x + n_cols - 1);
		// @TODO: updatnout size kontejneru s predchozim poctem elementu
	}

	for (int ix = x, ixe = x + n_cols; ix < ixe; ++ix)
	{
		//QModelIndex const idx0 = index(y, 0, QModelIndex());
		//setHeaderData(idx0, tr("%1").arg(y), Qt::EditRole);

		QModelIndex const idx = index(y, ix, QModelIndex());
		setData(idx, values.at(ix - x), Qt::EditRole);
	}
}

