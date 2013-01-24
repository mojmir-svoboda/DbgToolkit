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
	{
		if (role == Qt::BackgroundRole)
			return QVariant::fromValue<QColor>(Qt::gray);
		return QVariant();
	}

	if (c && (role == Qt::DisplayRole || role == Qt::ToolTipRole))
	{
		return c->m_value;
	}
	else if (role == Qt::BackgroundRole)
	{
		if (c->m_bgc.isValid())
			return c->m_bgc;
		if (c->m_value.toString().isEmpty())
			return QVariant::fromValue<QColor>(Qt::gray);
		return QVariant();
	}
	else if (role == Qt::ForegroundRole)
	{
		return c->m_fgc;
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
	if (orientation == Qt::Horizontal)
	{
		switch (role)
		{
			case Qt::DisplayRole:
				if (section > -1 && section < m_hhdr.size())
					return m_hhdr.at(section);
				break;

			// BLOODY HELL, Y U NO WORK?
			/*case Qt::SizeHintRole:
				if (section > -1 && section < m_hsize.size())
					return m_hsize.at(section);
				break;*/
		}
	}
	return QVariant();
}

bool  TableModelView::setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role)
{
	if (section == -1)
		return false;
	if (orientation == Qt::Horizontal)
	{
		if (role == Qt::EditRole)
		{
			if (section + 1 > m_hhdr.size())
			{
				m_hhdr.resize(section + 1);
			}
			m_hhdr[section] = value.toString();
			emit headerDataChanged(orientation, section, section);
		}
		else if (role == Qt::SizeHintRole)
		{
			if (section + 1 > m_hsize.size())
			{
				m_hsize.resize(section + 1);
			}
			m_hsize[section] = value.toInt();
			emit headerDataChanged(orientation, section, section);
		}
	}
	return true;
}

void TableModelView::emitLayoutChanged ()
{
	emit layoutChanged();
}

void TableModelView::appendTableXY (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & cmd)
{
	unsigned long long t = time.toULongLong();
	QStringList const values = cmd.split("|");
	int const n_cols = values.size();

	bool new_rows = false;
	int rows_first = 0;
	int rows_last = 0;

	bool new_cols = false;
	int cols_first = 0;
	int cols_last = 0;


/*	if (y < 0)
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
	else*/
	{
		if (y >= m_rows.size())
		{
			size_t const curr_sz = m_rows.size();
			m_rows.resize(y + 1);
			m_row_times.resize(y + 1);
			for (size_t r = curr_sz; r < y + 1; ++r)
				m_row_times[r] = t + r * 10;

			new_rows = true;
			rows_first = curr_sz;
			rows_last = y;
		}

/*		if (x < 0)
		{
			++m_columnCount;
			size_t const curr_sz = m_rows[y].size();
			m_rows[y].resize(curr_sz + 1);
			x = curr_sz;
			// @TODO: updatnout size kontejneru s predchozim poctem elementu
			//
		}
		else*/
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

	if (m_columnCount < x + n_cols)
	{
		new_cols = true;
		cols_first = m_columnCount;
		cols_last = x + n_cols - 1;
	}

	for (int ix = x, ixe = x + n_cols; ix < ixe; ++ix)
	{
		m_rows[y][ix].m_value = values.at(ix - x);
		m_rows[y][ix].m_fgc = fgc;
		m_rows[y][ix].m_bgc = bgc;
		m_col_times[ix] = t;
	}

	if (new_rows)
	{
		beginInsertRows(QModelIndex(), rows_first, rows_last);
		//qDebug("mod  -  beginInsertRows(%02i, %02i) ", rows_first, rows_last);
		endInsertRows();

	}

	if (new_cols)
	{
		beginInsertColumns(QModelIndex(), cols_first, cols_last);
		//qDebug("mod  |  beginInsertCols(%02i, %02i) ", cols_first, cols_last);
		insertColumns(cols_first, cols_last);
		if (m_columnCount < cols_last + 1)
			m_columnCount = cols_last + 1;
		endInsertColumns();
	}

	if (m_proxy && new_rows)
		m_proxy->insertRows(rows_first, rows_last);

	if (m_proxy && new_cols)
		m_proxy->insertColumns(cols_first, cols_last);

	for (int ix = x, ixe = x + n_cols; ix < ixe; ++ix)
	{
		QModelIndex const idx = index(y, ix, QModelIndex());
		emit dataChanged(idx, idx);
		if (m_proxy)
			m_proxy->setData(idx, QVariant(), Qt::EditRole);
	}
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
		endInsertRows();
	}
}

void TableModelView::createColumns (unsigned long long time, int first, int last, QModelIndex const & )
{
	if (last >= m_rows.back().size())
	{
		m_rows.back().resize(last);
		m_col_times.resize(last);
	}

	if (m_columnCount < last)
	{
		beginInsertColumns(QModelIndex(), m_columnCount, last - 1);
		insertColumns(m_columnCount, last - 1);
		m_columnCount = last;
		endInsertColumns();
	}

}


