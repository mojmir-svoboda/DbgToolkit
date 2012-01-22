#include "modelview.h"
#include <QBrush>
#include <QColor>
#include <QSortFilterProxyModel>
#include "Connection.h"
#include "../trace_client/trace.h"

ModelView::ModelView (QObject * parent, Connection * c)
	: QAbstractTableModel(parent)
	//, m_connection(c)
	, m_session_state(c->sessionState())
{
	size_t const prealloc_size = 128 * 1024;
	m_rows.reserve(prealloc_size); // @TODO: magic const!
}

ModelView::~ModelView ()
{
	qDebug("%s", __FUNCTION__);
}

int ModelView::rowCount (const QModelIndex & /*parent*/) const { return m_rows.size(); }

int ModelView::columnCount (const QModelIndex & /*parent*/) const
{
	if (m_session_state.getColumnsSetup())
		return m_session_state.getColumnsSetup()->size();
	return 0;
}

inline bool ModelView::checkExistence (QModelIndex const & index) const
{
	return index.row() < (int)m_rows.size() && index.column() < (int)m_rows[index.row()].size();
}

inline bool ModelView::checkColumnExistence (tlv::tag_t tag, QModelIndex const & index) const
{
	int const column_idx = m_session_state.findColumn4Tag(tag);
	return column_idx != -1 && column_idx == index.column() && checkExistence(index);
}

QVariant ModelView::data (const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (role == Qt::DisplayRole)
	{
		QString str("");
		if (checkExistence(index))
		{
			//qDebug("row=%u col=%u, rows_size=%u rows_size[%u].size()=%u value=%s",index.row(), index.column(), rows.size(), index.row(), rows[index.row()].size(), rows[index.row()][index.column()].toStdString().c_str());
			return m_rows[index.row()][index.column()];
		}
		return str;
	}
	if (role == Qt::BackgroundRole)
	{
		if (checkColumnExistence(tlv::tag_tid, index))
		{
			QString const & tid = m_rows[index.row()][index.column()];
			int const idx = m_session_state.getTLS().findThreadId(tid.toStdString());
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

QVariant ModelView::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) {
			if (m_session_state.getColumnsSetup() && 
					0 <= section && section < (int)m_session_state.getColumnsSetup()->size())
				return m_session_state.getColumnsSetup()->operator[](section);
		}
	}
	return QVariant();
}

void ModelView::transactionStart (size_t n)
{
	int const row = rowCount();
	beginInsertRows(QModelIndex(), row, row + n);

	emit layoutAboutToBeChanged();
}

void ModelView::transactionCommit ()
{
	endInsertRows();
	emit layoutChanged();
}

void ModelView::emitLayoutChanged ()
{
	emit layoutChanged();
}

void ModelView::appendCommand (QSortFilterProxyModel * filter, tlv::StringCommand const & cmd)
{
	int idx = -1;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
		if (cmd.tvs[i].m_tag == tlv::tag_tid)
			idx = m_session_state.getTLS().findThreadId(cmd.tvs[i].m_val);

	int indent = 0;
	if (idx >= 0)
		indent = m_session_state.getTLS().m_indents[idx];

	QString qindent;
	if (indent > 0)
		for(int j = 0; j < indent; ++j)
			qindent.append("  ");	// @TODO: ugh

	m_rows.push_back(columns_t(cmd.tvs.size()));
	columns_t & columns = m_rows.back();
	columns.reserve(cmd.tvs.size());

	if (cmd.hdr.cmd == tlv::cmd_scope_entry || (cmd.hdr.cmd == tlv::cmd_scope_exit))
		columns.resize(cmd.tvs.size() + 1);
	else
		columns.resize(cmd.tvs.size());

	std::string file;
	std::string line;
	std::string func;
	for (size_t i = 0, ie = cmd.tvs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = cmd.tvs[i].m_tag;
		std::string const & val = cmd.tvs[i].m_val;
		if (tag == tlv::tag_file)
			file = val;
		if (tag == tlv::tag_line)
			line = val;
		if (tag == tlv::tag_func)
			func = val;

		QString qval;
		if (tag == tlv::tag_msg)
		{
			qval.append(qindent);
		}

		qval.append(QString::fromStdString(val));
		int column_index = m_session_state.findColumn4Tag(tag);
		if (column_index < 0)
		{
			m_session_state.insertColumn();	// lines together
			column_index = m_session_state.getColumnsSetup()->size() - 1;
			columns.push_back(qval);		// keep these two
			char const * name = tlv::get_tag_name(tag);
			m_session_state.insertColumn4Tag(tag, column_index);
			if (name)
			{
				m_session_state.getColumnsSetup()->operator[](column_index) = QString::fromStdString(name);
			}
			else
			{
				m_session_state.getColumnsSetup()->operator[](column_index) = QString("???");
			}
		}
		columns[column_index] = qval;
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
			columns[column_index] = qindent_old + QString("{ ") + QString::fromStdString(func);
		}
	}

	if (cmd.hdr.cmd == tlv::cmd_scope_exit)
	{
		int column_index = m_session_state.findColumn4Tag(tlv::tag_msg);
		if (column_index >= 0)
		{
			columns[column_index] = qindent + QString("} ") + QString::fromStdString(func);
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
