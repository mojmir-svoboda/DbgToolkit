#include "tablewidget.h"
#include <QListView>
#include <QFile>
#include <QRegularExpression>
#include <tlv_parser/tlv_encoder.h>
#include "utils.h"
#include "utils_qstandarditem.h"
#include <filterproxymodel.h>
#include <treeproxy.h>
#include "logs/logtablemodel.h"
#include "qtsln/qtcolorpicker/qtcolorpicker.h"

namespace table {


void TableWidget::appendToTIDFilters (QString const & item)
{
	if (filterMgr()->getFilterTid())
	{
		QStandardItem * root = filterMgr()->getFilterTid()->m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, item);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addRow(item, true);
			root->appendRow(row_items);
			filterMgr()->getFilterTid()->append(item);
		}
	}
}

/*void TableWidget::appendToLvlWidgets (FilteredLevel const & flt)
{
	if (filterMgr()->getFilterLvl())
	{
		QStandardItem * root = filterMgr()->getFilterLvl()->m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, flt.m_level_str);
		if (child == 0)
		{
			E_LevelMode const mode = static_cast<E_LevelMode>(flt.m_state);
			QList<QStandardItem *> row_items = addTriRow(flt.m_level_str, Qt::Checked, lvlModToString(mode));
			row_items[0]->setCheckState(flt.m_is_enabled ? Qt::Checked : Qt::Unchecked);
			root->appendRow(row_items);
			filterMgr()->getFilterLvl()->m_ui->view->sortByColumn(0, Qt::AscendingOrder);
		}
	}
}*/

void TableWidget::appendToLvlFilters (QString const & item)
{
	if (filterMgr()->getFilterLvl())
	{
		bool enabled = false;
		E_LevelMode lvlmode = e_LvlInclude;
		if (filterMgr()->getFilterLvl()->isPresent(item, enabled, lvlmode))
			return;

		QStandardItem * root = filterMgr()->getFilterLvl()->m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, item);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addCheckableRow(Qt::Checked, item);
			root->appendRow(row_items);
			filterMgr()->getFilterLvl()->append(item);
		}
	}
}

/*void TableWidget::appendToCtxWidgets (FilteredContext const & flt)
{
	if (filterMgr()->getFilterCtx())
	{
		QStandardItem * root = filterMgr()->getFilterCtx()->m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, flt.m_ctx_str);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addRow(flt.m_ctx_str, true);
			row_items[0]->setCheckState(flt.m_is_enabled ? Qt::Checked : Qt::Unchecked);
			root->appendRow(row_items);
		}
	}
}*/


void TableWidget::appendToCtxFilters (QString const & item, bool checked)
{
	if (filterMgr()->getFilterCtx())
	{
		bool enabled = false;
		if (filterMgr()->getFilterCtx()->isCtxPresent(item, enabled))
			return;

		QStandardItem * root = filterMgr()->getFilterCtx()->m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, item);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addRow(item, true);
			row_items[0]->setCheckState(Qt::Checked);
			root->appendRow(row_items);
			filterMgr()->getFilterCtx()->appendCtxFilter(item);
		}
	}
}

void TableWidget::appendToFileLineFilters (QString const & item)
{
	if (filterMgr()->getFilterFileLine())
	{
		filterMgr()->getFilterFileLine()->fileModel()->insertItem(item);
	}
		//void const * node = filterMgr()->getFilterFileLine()->fileModel()->insertItem(file + "/" + line);
		//batch.m_tree_node_ptrs.back() = node;
	//if (ret.isValid())
		//->hideLinearParents();
}

bool TableWidget::appendToFilters (DecodedCommand const & cmd)
{
	/*QString tid;
	if (cmd.getString(tlv::tag_tid, tid))
	{
		int const idx = m_tls.findThreadId(tid);
		if (cmd.m_hdr.cmd == tlv::cmd_scope_entry)
			m_tls.incrIndent(idx);
		if (cmd.m_hdr.cmd == tlv::cmd_scope_exit)
			m_tls.decrIndent(idx);
		appendToTIDFilters(tid);
	}*/

	QString ctx;
	if (cmd.getString(tlv::tag_ctx, ctx))
		appendToCtxFilters(ctx, false);

	QString lvl;
	if (cmd.getString(tlv::tag_lvl, lvl))
		appendToLvlFilters(lvl);

	QString file, line;
	if (cmd.getString(tlv::tag_line, line) && cmd.getString(tlv::tag_file, file))
		appendToFileLineFilters(file + "/" + line);

	return true;
}

void TableWidget::appendToRegexFilters (QString const & str, bool checked, bool inclusive)
{
	if (filterMgr()->getFilterRegex())
		filterMgr()->getFilterRegex()->appendToRegexFilters(str, checked, inclusive);
}

void TableWidget::removeFromRegexFilters (QString const & val)
{
	if (filterMgr()->getFilterRegex())
		filterMgr()->getFilterRegex()->removeFromRegexFilters(val);
}

void TableWidget::appendToStringFilters (QString const & str, bool checked, int state)
{
	if (filterMgr()->getFilterString())
	{
		filterMgr()->getFilterString()->appendToStringFilters(str, checked, state);
	}
}

void TableWidget::removeFromStringFilters (QString const & val)
{
	if (filterMgr()->getFilterString())
	{
		filterMgr()->getFilterString()->removeFromStringFilters(val);
	}
}

}

