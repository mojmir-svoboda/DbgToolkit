#include "logtablemodel.h"
#include <QBrush>
#include <QColor>
#include <QAbstractProxyModel>
#include <connection.h>
#include <models/filterproxymodel.h>
#include "findproxymodel.h"
#include <utils/utils.h>
#include <sysfn/time_query.h>

namespace logs {

LogTableModel::LogTableModel (QObject * parent, logs::LogWidget & lw)
	: BaseTableModel(parent, lw.m_config.m_columns_setup, lw.m_config.m_columns_sizes)
	, m_log_widget(lw)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);

	m_data.init();

	beginInsertColumns(QModelIndex(), 0, columnCount() - 1);
	insertColumns(0, columnCount() - 1);
	endInsertColumns();
}

LogTableModel::~LogTableModel ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

int LogTableModel::rowCount (QModelIndex const & /*parent*/) const
{
	int const sz = m_data.rowCount();
	return sz;
}

int LogTableModel::columnCount (QModelIndex const & /*parent*/) const
{
	int const sz = m_data.columnCount();
	return sz;
}

inline bool LogTableModel::checkExistence (QModelIndex const & index) const
{
	Q_ASSERT(index.row() != -1 && index.column() != -1);
	return index.row() < rowCount() && index.column() < columnCount();
}

QVariant LogTableModel::data (QModelIndex const & index, int role) const
{
	if (!index.isValid())
		return QVariant();

	int const col = index.column();
	int const row = index.row();
	proto::Record const * rec = nullptr;
	proto::Attrs const * attrs = nullptr;
	if (checkExistence(index))
	{
		rec = m_data.getRecord(row);
		attrs = m_data.getAttrs(row);
	}

	if (rec == nullptr)
	{
		if (role == Qt::BackgroundRole)
			return QVariant::fromValue<QColor>(Qt::gray);
		return QVariant();
	}

	if (role == Qt::DisplayRole)
	{
		QVariant const ret = m_data.getRecordData(rec->m_values, col);
		return ret;
	}
	else if (role == Qt::ToolTipRole)
	{
		QVariant const ret = m_data.getRecordData(rec->m_values, col);
		QString ttip("<pre>");
		ttip.append(ret.toString());
		ttip.append(QString("</pre>"));
		return ttip;
	}
	else if (role == Qt::BackgroundRole)
	{
// 		QVariant const col_flags_qv = m_data.getRecordData(attrs->m_flags, col);
// 		proto::Flags col_flags = col_flags_qv.value<proto::Flags>();
// 
// 		if (col_flags.m_scope_type == LogScopeType_scopeEntry)
// 			return QVariant::fromValue<QColor>(QColor(0xEFEDFC));
// 		if (col_flags.m_scope_type == LogScopeType_scopeExit)
// 			return QVariant::fromValue<QColor>(QColor(0xEAF1FB));
		
		QVariant const col_rgba = m_data.getRecordData(attrs->m_bgcols, col);
		if (col_rgba.isValid())
			if (unsigned const rgba = col_rgba.toUInt())
				return QVariant::fromValue<QColor>(QColor(rgba));
	}
	else if (role == Qt::ForegroundRole)
	{
		QVariant const col_rgba = m_data.getRecordData(attrs->m_fgcols, col);
		if (col_rgba.isValid())
			if (unsigned const rgba = col_rgba.toUInt())
				return QVariant::fromValue<QColor>(QColor(rgba));
	}
	return QVariant();
}

bool LogTableModel::setData (QModelIndex const & index, QVariant const & value, int role)
{
	if (!index.isValid()) return false;
	if (!checkExistence(index)) return false;

	int const x = index.column();
	int const y = index.row();
	if (role == Qt::DisplayRole)
		return false;
	else if (role == Qt::EditRole)
	{
		//m_rows.at(y).at(x).m_value = value;
	}
	else if (role == Qt::BackgroundRole)
	{
		QVariant const bg_val = value;
		QColor const col = bg_val.value<QColor>();
		qtcolor const qtcol = col.rgba();

		proto::rw_rowdata_t rowdata = m_data.getRecordDataForRowReadWrite(index.row());
		proto::Attrs * attrs = std::get<1>(rowdata);
		m_data.setRecordData(attrs->m_bgcols, index.column(), qtcol);
	}
	else if (role == Qt::ForegroundRole)
	{
		QVariant const fg_val = value;
		QColor const col = fg_val.value<QColor>();
		qtcolor const qtcol = col.rgba();

		proto::rw_rowdata_t rowdata = m_data.getRecordDataForRowReadWrite(index.row());
		proto::Attrs * attrs = std::get<1>(rowdata);
		m_data.setRecordData(attrs->m_fgcols, index.column(), qtcol);
	}

	emit dataChanged(index, index);
	return true;
}

QVariant LogTableModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal)
	{
		switch (role)
		{
			case Qt::DisplayRole:
				if (section > -1 && section < std::tuple_size<proto::values_t>::value)
				{
					proto::tags const tag = proto::getTagForCol(section);
					return proto::get_tag_name(tag);
				}
				break;
			case Qt::SizeHintRole:
				if (section > -1 && section < std::tuple_size<proto::values_t>::value)
				{
					proto::tags const tag = proto::getTagForCol(section);
					size_t const sz = m_log_widget.m_config.m_tag_config.findTag(tag).m_size;
					return QSize(sz, 20);
				}
				return QSize(64, 20);
		}
	}
	return QVariant();
}

bool LogTableModel::setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role)
{
	return true;
}

void LogTableModel::commitCommands (std::vector<DecodedCommand> const & dcmds, E_ReceiveMode mode)
{
	size_t const rows = dcmds.size();
	size_t const from = m_data.getRecordCount();
	int const to = static_cast<int>(from) + static_cast<int>(rows) - 1;

	if (rows == 0 || from == -1 || to == -1)
		return;

	bool signal_model_reset = false;
	{
		beginInsertRows(QModelIndex(), static_cast<int>(from), to);

		for (size_t r = 0, re = rows; r < re; ++r)
		{
			DecodedCommand const & dcmd = dcmds[r];
			m_data.handleLogCommand(dcmd, m_log_widget.m_config);
		}

		endInsertRows();
	}

	if (from == 0 && rowCount() > 0)
		m_log_widget.m_tableview->setCurrentIndex(index(0, 0, QModelIndex()));

	postProcessBatch(static_cast<int>(from), static_cast<int>(from) + static_cast<int>(rows));
}

void LogTableModel::postProcessBatch (int src_from, int src_to)
{
	int const rows = src_to - src_from;
	for (int src_row = src_from; src_row < src_to; ++src_row)
	{
		QModelIndex const src_idx = m_log_widget.m_src_model->index(src_row, 0, QModelIndex());
		m_log_widget.processByColorizers(src_idx);
		m_log_widget.processBySounds(src_idx);
	}

	for (BaseProxyModel * pxy : m_proxy)
	{
		pxy->commitBatchToModel(src_from, src_to);
	}
}

void LogTableModel::reloadModelAccordingTo (logs::LogConfig & config)
{
// 	resizeToCfg(config);
// 	for (size_t r = 0, re = m_dcmds.size(); r < re; ++r)
// 	{
// 		DecodedCommand const & dcmd = m_dcmds[r];
// 		handleCommand(dcmd, e_RecvBatched);
// 	}
// 	commitCommands(e_RecvSync);
//   //emit headerDataChanged(Qt::Horizontal, 0, 10);// TODO TODO TODO TST
}

/*LogTableModel * LogTableModel::cloneToNewModel ()
{
	LogTableModel * new_model = new LogTableModel(this, m_log_widget);
	new_model->m_rows = m_rows;
	new_model->m_dcmds = m_dcmds;

	new_model->m_row_ctimes = m_row_ctimes;
	new_model->m_row_stimes = m_row_stimes;
	new_model->m_col_times = m_col_times;
	new_model->m_column_count = m_column_count;
	return new_model;
}*/
LogTableModel * LogTableModel::cloneToNewModel (logs::LogWidget * parent, FindConfig const & fc)
{
	LogTableModel * new_model = new LogTableModel(parent, *parent);
// 	for (size_t r = 0, re = m_rows.size(); r < re; ++r)
// 	{
// 		DecodedCommand const & dcmd = m_dcmds[r];
// 		bool row_match = false;
// 		for (size_t i = 0, ie = dcmd.m_tvs.size(); i < ie; ++i)
// 		{
// 			QString const & val = dcmd.m_tvs[i].m_val;
// 
// 			if (matchToFindConfig(val, fc))
// 			{
// 				row_match = true;
// 				break;
// 			}
// 		}
// 
// 		if (row_match)
// 		{
// 			new_model->m_rows.push_back(m_rows[r]);
// 			new_model->m_dcmds.push_back(m_dcmds[r]);
// 			new_model->m_row_ctimes.push_back(m_row_ctimes[r]);
// 			new_model->m_row_stimes.push_back(m_row_stimes[r]);
// 		}
// 	}
// 	new_model->m_col_times = m_col_times;
	return new_model;
}

void LogTableModel::clearModelData ()
{
	beginResetModel();

	removeRows(0, rowCount());
	m_data.clear();

	endResetModel();
}

void LogTableModel::clearModel ()
{
	BaseTableModel::clearModel();
}


/*
QVariant LogTableModel::data (const QModelIndex &index, int role) const
{
  bool const is_collapsed = m_filter_state.isBlockCollapsedIncl(tid, index.row());
  if (is_collapsed)
    return QBrush(Qt::lightGray);

  QString const & lvl = m_rows[index.row()][index.column()];
  if (lvl.toInt() == trace::e_Fatal)
    return QBrush(Qt::white);

	return QVariant();
}

QVariant LogTableModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) {
			if (0 <= section && section < (int)m_log_widget.m_config.m_columns_setup.size())
				return m_log_widget.m_config.m_columns_setup[section];
		}
	}
	return QVariant();
}
*/

}
