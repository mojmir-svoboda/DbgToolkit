#include "filterproxy.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QListView>
#include <tlv_parser/tlv_encoder.h>
#include <trace_client/trace.h>
//#include "mainwindow.h"

void FilterProxyModel::force_update ()
{
	reset();
}

bool FilterProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & /*sourceParent*/) const
{
	//MainWindow const * mw = static_cast<Connection const *>(parent())->getMainWindow();

	QString file, line;
	int const col_idx = m_session_state.findColumn4Tag(tlv::tag_file);
	if (col_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, col_idx, QModelIndex());
		file = sourceModel()->data(data_idx).toString();
	}
	int const col_idx2 = m_session_state.findColumn4Tag(tlv::tag_line);
	if (col_idx2 >= 0)
	{
		QModelIndex data_idx2 = sourceModel()->index(sourceRow, col_idx2, QModelIndex());
		line = sourceModel()->data(data_idx2).toString();
	}

	bool excluded = false;
	excluded |= m_session_state.isFileLineExcluded(std::make_pair(file.toStdString(), line.toStdString()));

	QString tid;
	int const tid_idx = m_session_state.findColumn4Tag(tlv::tag_tid);
	if (tid_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, tid_idx, QModelIndex());
		tid = sourceModel()->data(data_idx).toString();
	}

	QString ctx;
	int const ctx_idx = m_session_state.findColumn4Tag(tlv::tag_ctx);
	if (ctx_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, ctx_idx, QModelIndex());
		ctx = sourceModel()->data(data_idx).toString();
	}

	bool regex_accept = true;
	if (m_regexps.size() > 0)
	{
		regex_accept = false;
		QString msg;
		int const msg_idx = m_session_state.findColumn4Tag(tlv::tag_msg);
		if (msg_idx >= 0)
		{
			QModelIndex data_idx = sourceModel()->index(sourceRow, msg_idx, QModelIndex());
			msg = sourceModel()->data(data_idx).toString();
		}

		for (int i = 0, ie = m_regexps.size(); i < ie; ++i)
		{
			if (m_regex_user_states[i])
				regex_accept |= m_regexps[i].exactMatch(msg);
		}
	}

	excluded |= m_session_state.isTIDExcluded(tid.toStdString());
	excluded |= m_session_state.isCtxExcluded(ctx.toULongLong());

	QModelIndex data_idx = sourceModel()->index(sourceRow, 0, QModelIndex());
	excluded |= m_session_state.isBlockCollapsed(tid, data_idx.row());
	excluded |= data_idx.row() < m_session_state.excludeContentToRow();
	return !excluded && regex_accept;
}

