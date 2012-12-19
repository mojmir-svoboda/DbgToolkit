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

	Cell const * c = 0;
	if (checkExistence(index))
	{
		c = &m_rows[index.row()][index.column()];
	}

	if (c == 0)
		return QVariant();

	if (c && (role == Qt::DisplayRole || role == Qt::ToolTipRole))
	{
		return c->m_value;
	}
	else if (role == Qt::BackgroundRole)
	{
		if (c->m_bgc.value<QColor>().isValid())
			return c->m_bgc;
		if (c->m_value.toString().isEmpty())
			return Qt::gray;
	}
	else if (role == Qt::ForegroundRole)
	{
		return c->m_fgc.value<QColor>().isValid() ? c->m_fgc : QVariant();
	}
	return QVariant();
}

bool TableModelView::setData (QModelIndex const & index, QVariant const & value, int role)
{
	if (!index.isValid()) return false;
	if (!checkExistence(index)) return false;

	int const x = index.column();
	int const y = index.row();
	if (role == Qt::DisplayRole)
		return false;
	else if (role == Qt::EditRole)
	{
		m_rows.at(y).at(x).m_value = value;
	}
	else if (role == Qt::BackgroundRole)
	{
		m_rows.at(y).at(x).m_bgc = value;
	}
	else if (role == Qt::ForegroundRole)
	{
		m_rows.at(y).at(x).m_fgc = value;
	}

	emit dataChanged(index, index);
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

		if (!m_hhdr[section].isEmpty())
		{
			qDebug("hdr[%i] ret name=%s", section, m_hhdr[section].toStdString().c_str());
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
			m_hhdr.resize(section + 1);
		}
		m_hhdr[section] = value.toString();
		if (!m_hhdr[section].isEmpty())
		{
			qDebug("hdr[%i] setting name=%s", section, m_hhdr[section].toStdString().c_str());
		}
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

void TableModelView::appendTableXY (int x, int y, QString const & time, QString const & cmd)
{
	unsigned long long t = time.toULongLong();
	QStringList const values = cmd.split("|");
	int const n_cols = values.size();

	//qDebug("append: x=%i y=%i cols=%i val=%s", x, y, n_cols, cmd.toStdString().c_str());
	//
	bool pxy_rows = false;
	size_t rows_first = 0;
	size_t rows_last = 0;

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
			//qDebug("+ model: y>rows.sz resize m_rows.sz=%i y=%i", m_rows.size(), y);
			beginInsertRows(QModelIndex(), m_rows.size(), y);
			size_t const curr_sz = m_rows.size();
			m_rows.resize(y + 1);
			m_row_times.resize(y + 1);
			for (size_t r = curr_sz; r < y + 1; ++r)
				m_row_times[r] = t + r * 10;
			transactionCommit();

			pxy_rows = true;
			rows_first = curr_sz;
			rows_last = y;
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
				m_col_times.resize(x + n_cols);
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
		QString const & s = values.at(ix - x);
		setData(idx, s, Qt::EditRole);
		m_col_times[ix] = t;
	}

	if (m_proxy && pxy_rows)
		m_proxy->insertRows(rows_first, rows_last);
}

void TableModelView::createCell (unsigned long long time, int x, int y)
{
}

void TableModelView::createRows (unsigned long long time, int first, int last, QModelIndex const &)
{
	if (first >= m_rows.size())
	{
		//qDebug("+ model: y>rows.sz resize m_rows.sz=%i y=%i", m_rows.size(), first);
		beginInsertRows(QModelIndex(), m_rows.size(), first);
		size_t const curr_sz = m_rows.size();
		m_rows.resize(last + 1);
		m_row_times.resize(last + 1);
		for (size_t r = curr_sz; r < last + 1; ++r)
			m_row_times[r] = time + r * 10;
		transactionCommit();
	}
}

void TableModelView::createColumns (unsigned long long time, int first, int last, QModelIndex const & )
{
	//qDebug("  append: x>=0 ", m_rows.back().size(), x + n_cols);
	if (last >= m_rows.back().size())
	{
		//qDebug("  append: x>=0  resize %i -> %i", m_rows.back().size(), x + n_cols);
		m_rows.back().resize(last);
		m_col_times.resize(last);
	}

	//qDebug("  m_columnCount < n_cols  %i < %i", m_columnCount, x + n_cols);
	if (m_columnCount < last)
	{
		beginInsertColumns(QModelIndex(), m_columnCount, last);
		insertColumns(m_columnCount, last);
		m_columnCount = last + 1;
		endInsertColumns();

		//if (m_proxy)
		//	m_proxy->insertColumns(m_columnCount, last - 1);
		// @TODO: updatnout size kontejneru s predchozim poctem elementu
	}

}


