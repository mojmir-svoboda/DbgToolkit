#include "tablemodelview.h"
#include <QBrush>
#include <QColor>
#include <QAbstractProxyModel>
#include <trace_client/trace.h>

TableModelView::TableModelView (QObject * parent)
	: QAbstractTableModel(parent)
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
	if (m_rows.size() > 0)
		return m_rows[0].size();
	return 0;
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


bool TableModelView::setData (QModelIndex const & /*index*/, QVariant const & /*value*/, int /*role*/)
{
	return true;	
}

QVariant TableModelView::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) {
		}
	}
	return QVariant();
}

void TableModelView::transactionStart (size_t n)
{
	int const row = rowCount();
	beginInsertRows(QModelIndex(), row, row + n);
	//emit layoutAboutToBeChanged();
}

void TableModelView::transactionCommit ()
{
	endInsertRows();
	//emit layoutChanged();
}

void TableModelView::emitLayoutChanged ()
{
	emit layoutChanged();
}

void TableModelView::appendTableXY (int x, int y, QString const & msg_tag)
{
	if (y < 0)
	{
		m_rows.push_back(columns_t());
		if (x < 0)
		{
			m_rows.back().push_back("aa");
		}
		else
			m_rows.back().resize(x + 1);
	}
	else
	{
		m_rows.resize(y + 1);
		if (x < 0)
		{
			m_rows.back().push_back("bb");
		}
		else
			m_rows.back().resize(x + 1);

	}
}

/*void TableModelView::appendCommand (tlv::StringCommand const & cmd)
{
	int column_index = -1;

	m_rows.push_back(columns_t(cmd.tvs.size()));
	columns_t & columns = m_rows.back();
	columns.reserve(cmd.tvs.size());

	size_t n = cmd.tvs.size();
	columns.resize(n);
	std::string msg;
	for (size_t i = 0, ie = cmd.tvs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = cmd.tvs[i].m_tag;
		std::string const & val = cmd.tvs[i].m_val;
		if (tag == tlv::tag_msg)
			msg = val;

		columns[column_index] = msg;
	}

	//if (m_main_window->dtEnabled())
	{
		int const tag = tlv::tag_max_value + 1;
		int ci = m_session_state.findColumn4Tag(static_cast<tlv::tag_t>(tag));
		if (ci < 0)
		{
			ci = m_session_state.insertColumn(tag);
			if (filter)
			{
				filter->insertColumn(ci);
			}
		}
	}

	int const row = rowCount();
	insertRow(row);
}*/

