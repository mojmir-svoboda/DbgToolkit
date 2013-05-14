#include "logtablemodel.h"
#include <QBrush>
#include <QColor>
#include <QAbstractProxyModel>
#include "connection.h"
#include <trace_client/trace.h>

LogTableModel::LogTableModel (QObject * parent, Connection * c)
	: QAbstractTableModel(parent)
	, m_connection(c)
	, m_session_state(c->sessionState())
{
	size_t const prealloc_size = 128 * 1024;
	m_rows.reserve(prealloc_size); // @TODO: magic const!
}

LogTableModel::~LogTableModel ()
{
	qDebug("%s", __FUNCTION__);
}

int LogTableModel::rowCount (const QModelIndex & /*parent*/) const { return static_cast<int>(m_rows.size()); }

int LogTableModel::columnCount (const QModelIndex & /*parent*/) const
{
	if (m_session_state.getColumnsSetupCurrent())
		return m_session_state.getColumnsSetupCurrent()->size();
	return 0;
}

inline bool LogTableModel::checkExistence (QModelIndex const & index) const
{
	return index.row() < (int)m_rows.size() && index.column() < (int)m_rows[index.row()].size();
}

inline bool LogTableModel::checkTagExistence (tlv::tag_t tag, QModelIndex const & index) const
{
	int const column_idx = m_session_state.findColumn4Tag(tag);
	return column_idx != -1 && column_idx == index.column();
}

inline bool LogTableModel::checkColumnExistence (tlv::tag_t tag, QModelIndex const & index) const
{
	int const column_idx = m_session_state.findColumn4Tag(tag);
	return column_idx != -1 && column_idx == index.column() && checkExistence(index);
}

QVariant LogTableModel::data (const QModelIndex &index, int role) const
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

	bool const color_set = (role == Qt::BackgroundRole || role == Qt::ForegroundRole);
	if (color_set)
	{
		if (checkColumnExistence(tlv::tag_msg, index))
		{
			QString const & msg = m_rows[index.row()][index.column()];

			QColor color;
			E_ColorRole color_role = e_Bg;
			bool const is_match = m_session_state.isMatchedColorizedText(msg, color, color_role);

			if (is_match && role == Qt::BackgroundRole && color_role == e_Bg)
			{
				return QBrush(color);
			}
			if (is_match && role == Qt::ForegroundRole && color_role == e_Fg)
			{
				return QBrush(color);
			}
		}
	}

	if (role == Qt::BackgroundRole)
	{
		if (checkColumnExistence(tlv::tag_msg, index))
		{
			int const column_idx = m_session_state.findColumn4Tag(tlv::tag_tid);
			if (column_idx != -1)
			{
				QString const & tid = m_rows[index.row()][column_idx];

				bool const is_collapsed = m_session_state.isBlockCollapsedIncl(tid, index.row());
				if (is_collapsed)
					return QBrush(Qt::lightGray);
			}
		}

		if (checkColumnExistence(tlv::tag_tid, index))
		{
			QString const & tid = m_rows[index.row()][index.column()];
			int const idx = m_session_state.getTLS().findThreadId(tid);
			if (idx >= 0)
				return QBrush(m_session_state.getThreadColors()[idx]);
		}
		if (checkColumnExistence(tlv::tag_lvl, index))
		{
			QString const & lvl = m_rows[index.row()][index.column()];
            if (lvl.toInt() == trace::e_Fatal)
				return QBrush(Qt::black);
			if (lvl.toInt() == trace::e_Error)
				return QBrush(Qt::red);
			if (lvl.toInt() == trace::e_Warning)
				return QBrush(Qt::yellow);
		}

	}
	if (role == Qt::ForegroundRole)
	{
		if (checkColumnExistence(tlv::tag_lvl, index))
		{
			QString const & lvl = m_rows[index.row()][index.column()];
			if (lvl.toInt() == trace::e_Fatal)
				return QBrush(Qt::white);
		}
	}

	return QVariant();
}


bool LogTableModel::setData (QModelIndex const & index, QVariant const & value, int role)
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

QVariant LogTableModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) {
			if (m_session_state.getColumnsSetupCurrent() && 
					0 <= section && section < (int)m_session_state.getColumnsSetupCurrent()->size())
				return m_session_state.getColumnsSetupCurrent()->operator[](section);
		}
	}
	return QVariant();
}

void LogTableModel::transactionStart (int n)
{
	int const row = rowCount();
	beginInsertRows(QModelIndex(), row, row + n);

	//emit layoutAboutToBeChanged();
}

void LogTableModel::transactionCommit ()
{
	endInsertRows();
	//emit layoutChanged();
}

void LogTableModel::emitLayoutChanged ()
{
	emit layoutChanged();
}

void LogTableModel::appendCommand (QAbstractProxyModel * filter, tlv::StringCommand const & cmd)
{
	int column_index = -1;
	int thread_idx = -1;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
		if (cmd.tvs[i].m_tag == tlv::tag_tid)
			thread_idx = m_session_state.getTLS().findThreadId(cmd.tvs[i].m_val);

	int indent = 0;
	QString qindent;
	if (m_connection->getMainWindow()->indentEnabled())
	{
		if (thread_idx >= 0)
			indent = m_session_state.getTLS().m_indents[thread_idx];

		if (indent > 0)
			for(int j = 0; j < indent; ++j)
				qindent.append("  ");	// @TODO: ugh
	}

	m_rows.push_back(columns_t(cmd.tvs.size()));
	m_layers.push_back(indent);
	m_rowTypes.push_back(cmd.hdr.cmd);
	columns_t & columns = m_rows.back();
	columns.reserve(cmd.tvs.size());

	size_t n = cmd.tvs.size();
	if (cmd.hdr.cmd == tlv::cmd_scope_entry || (cmd.hdr.cmd == tlv::cmd_scope_exit))
		n = n + 1;
	
	//if (m_main_window->dtEnabled())
		n = n + 1;

	columns.resize(n);

	QString file;
	QString line;
	QString func;
	QString time;
	QString msg;
	for (size_t i = 0, ie = cmd.tvs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = cmd.tvs[i].m_tag;
		QString const & val = cmd.tvs[i].m_val;
		if (tag == tlv::tag_file)
			file = val;
		if (tag == tlv::tag_line)
			line = val;
		if (tag == tlv::tag_func)
			func = val;
		if (tag == tlv::tag_time)
			time = val;

		QString qval;
		if (tag == tlv::tag_msg)
		{
			msg = val;
			qval.append(qindent);
		}

		qval.append(val);
		column_index = m_session_state.findColumn4Tag(tag);
		if (column_index < 0)
		{
			column_index = m_session_state.insertColumn(tag);

			if (filter)
			{
				filter->insertColumn(column_index);
			}
		}
		columns[column_index] = qval;
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

		unsigned long long const last_t = m_session_state.getTLS().lastTime(thread_idx);
		unsigned long long const t = time.toULongLong();
		long long const dt = t - last_t;
		columns[ci] = tr("%1").arg(dt);
		m_session_state.getTLS().setLastTime(thread_idx, t);
	}

	if (cmd.hdr.cmd == tlv::cmd_scope_entry)
	{
		int column_index = m_session_state.findColumn4Tag(tlv::tag_msg);
		if (column_index >= 0)
		{
			QString qindent_old;
			if (indent > 1)
				for(int j = 0; j < indent - 1; ++j)
					qindent_old.append("  "); // @TODO: ugh
			columns[column_index] = qindent_old + QString("{ ") + msg;
		}
	}

	if (cmd.hdr.cmd == tlv::cmd_scope_exit)
	{
		int column_index = m_session_state.findColumn4Tag(tlv::tag_msg);
		if (column_index >= 0)
		{
			columns[column_index] = qindent + QString("} ") + msg;
		}
	}

	if (filter)
	{
		int const row = filter->rowCount();
		filter->insertRow(row);
	}
	else
	{
		int const row = rowCount();
		insertRow(row);
	}
}

void LogTableModel::appendCommandCSV (QAbstractProxyModel * filter, tlv::StringCommand const & cmd)
{
	m_rows.push_back(columns_t(cmd.tvs.size()));

	QString msg;
	for (size_t i = 0, ie = cmd.tvs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = cmd.tvs[i].m_tag;
		QString const & val = cmd.tvs[i].m_val;
		if (tag == tlv::tag_msg)
			msg = val;
	}

	//QStringList list = msg.split(QRegExp(separator), QString::SkipEmptyParts);
	QStringList const list = msg.split(m_session_state.separatorChar());
	columns_t & columns = m_rows.back();
	columns.resize(list.size());
	for (int i = 0, ie = list.size(); i < ie; ++i)
	{
		QString const & col_value = list.at(i);
		int column_index = m_session_state.findColumn4Tag(i);
		if (column_index < 0)
		{
			column_index = m_session_state.insertColumn(i);
			beginInsertColumns(QModelIndex(), column_index, column_index + 3);
			insertColumns(column_index, 3);
			endInsertColumns();

			if (filter)
			{
				filter->insertColumn(column_index);
			}
		}

		//QModelIndex const idx = createIndex(m_rows.size() - 1, column_index, 0);
		QModelIndex const idx = index(m_rows.size() - 1, column_index, QModelIndex());
		setData(idx, col_value, Qt::EditRole);
	}

	/*m_table_view_widget->horizontalHeader()->resizeSections(QHeaderView::Fixed);
	m_table_view_widget->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
	m_table_view_widget->verticalHeader()->setResizeMode(QHeaderView::Fixed);
	m_table_view_widget->setVisible(true);*/


	if (filter)
	{
		int const row = filter->rowCount();
		filter->insertRow(row);
	}
	else
	{
		int const row = rowCount();
		insertRow(row);
	}
}

