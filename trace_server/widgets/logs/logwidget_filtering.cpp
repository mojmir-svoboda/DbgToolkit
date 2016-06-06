#include "logwidget.h"
#include <QListView>
#include <QFile>
#include <QRegularExpression>
#include <utils/utils.h>
#include <utils/utils_qstandarditem.h>
#include "logfilterproxymodel.h"
#include <models/treeproxy.h>
#include "logtablemodel.h"
#include <3rd/qtsln/qtcolorpicker/qtcolorpicker.h>

namespace logs {

void LogWidget::onInvalidateFilter ()
{
	QItemSelectionModel const * selection = m_tableview->selectionModel();
	if (!selection)
		return;

	QModelIndexList const old_selection = selection->selectedIndexes();

	//  fantomas ended here
	QModelIndexList srcs;
	if (isModelProxy())
	{
		for (int i = 0, ie = old_selection.size(); i < ie; ++i)
		{
			QModelIndex const & pxy_idx = old_selection.at(i);
			QModelIndex const src_idx = m_proxy_model->mapToSource(pxy_idx);

			//qDebug("update filter: pxy=(%2i, %2i) src=(%2i, %2i)", pxy_idx.row(), pxy_idx.column(), src_idx.row(), src_idx.column());
			srcs.push_back(src_idx);
		}
	}
	else
		srcs = old_selection;

	if (isModelProxy())
		static_cast<FilterProxyModel *>(m_proxy_model)->force_update();
	else
	{
		m_src_model->emitLayoutChanged();
	}

	syncSelection(srcs);

	scrollToCurrentTagOrSelection();
}

void LogWidget::syncSelection (QModelIndexList const & sel)
{
	m_tableview->selectionModel()->clearSelection();

	for (int i = 0, ie = sel.size(); i < ie; ++i)
	{
		QModelIndex idx = sel.at(i);
		if (isModelProxy())
		{
			idx = m_proxy_model->mapFromSource(sel.at(i));
			//qDebug("syncSelection: pxy src=(%2i, %2i)", idx.row(), idx.column());
		}
		//else qDebug("syncSelection: src=(%2i, %2i)", idx.row(), idx.column());

		if (idx.isValid())
			m_tableview->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Select);
	}
}

/*void LogWidget::setFindProxyModel (FindConfig const & fc)
{
	m_config.m_find_config = fc;
	setModel(m_find_proxy_model);
	m_find_proxy_model->force_update();
	m_find_proxy_model->resizeToCfg();
	resizeSections();
	applyConfig();
}*/


void LogWidget::setFilteringProxy (bool on)
{
	//////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	//on = false;
	//////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	QItemSelectionModel const * selection = m_tableview->selectionModel();
	QModelIndexList indexes;
	if (selection)
		selection->selectedIndexes();

	if (!on)
	{
		qDebug("%s setting source model", __FUNCTION__);
		QModelIndexList srcs;
		if (m_proxy_model && indexes.size() > 0)
		{
			for (int i = 0, ie = indexes.size(); i < ie; ++i)
			{
				QModelIndex const & pxy_idx = indexes.at(i);
				QModelIndex const src_idx = m_proxy_model->mapToSource(pxy_idx);
				srcs.push_back(src_idx);
			}
		}

		m_tableview->setModel(m_src_model);

		if (srcs.size() > 0)
			m_tableview->setSelectionModel(m_src_selection);

		m_src_model->removeProxy(m_proxy_model); //m_src_model->setProxy(0);

		for (int i = 0, ie = srcs.size(); i < ie; ++i)
			m_tableview->selectionModel()->setCurrentIndex(srcs.at(i), QItemSelectionModel::Select);
	}
	else
	{
		qDebug("%s setting proxy model", __FUNCTION__);
		m_tableview->setModel(m_proxy_model);
		// selection
		m_src_model->removeProxy(m_proxy_model);
		m_src_model->addProxy(m_proxy_model);

		if (m_proxy_model)
			m_proxy_model->setSourceModel(m_src_model);

		if (m_proxy_model)
			m_proxy_model->force_update();

		m_tableview->setSelectionModel(m_kproxy_selection);

		QModelIndexList pxys;
		for (int i = 0, ie = indexes.size(); i < ie; ++i)
		{
			QModelIndex const & src_idx = indexes.at(i);
			QModelIndex const pxy_idx = m_proxy_model->mapFromSource(src_idx);
			//qDebug("on: src(r=%i c=%i) -> pxy(r=%i c=%i)", src_idx.row(), src_idx.column(), pxy_idx.row(), pxy_idx.column());
			if (pxy_idx.isValid())
				pxys.push_back(pxy_idx);
		}

		for (int i = 0, ie = pxys.size(); i < ie; ++i)
		{
			//qDebug("on: pxy r=%i c=%i", pxys.at(i).row(), pxys.at(i).column());
			m_tableview->selectionModel()->setCurrentIndex(pxys.at(i), QItemSelectionModel::Select);
		}
	}

	if (m_config.m_in_view)
		scrollToCurrentTagOrSelection();
		//QTimer::singleShot(1000, this, SLOT(scrollToCurrentTagOrSelection()));
}

/*void LogWidget::clearFilters (QStandardItem * node)
{
	if (node)
	{
		if (node->checkState() == Qt::Checked)
		{
			node->setCheckState(Qt::Unchecked);
		}
		for (int i = 0, ie = node->rowCount(); i < ie; ++i)
			clearFilters(node->child(i));
	}
}*/

void LogWidget::refreshFilters (BaseProxyModel const * proxy)
{
// 	QAbstractItemModel * m = m_tableview->model();
// 	if (m && m == proxy)
// 	{
// 		Q_ASSERT(m_src_model);
// 
// 		QModelIndexList src_list;
// 		for (int i = 0, ie = m_tableview->model()->rowCount(); i < ie; ++i)
// 		{
// 			QModelIndex const pxy_idx = m_tableview->model()->index(i, 0);
// 			QModelIndex const src_idx = proxy->mapToSource(pxy_idx);
// 
// 			if (src_idx.isValid())
// 				src_list << src_idx;
// 		}
// 		foreach(QModelIndex index, src_list)
// 		{
// 			DecodedCommand const & dcmd = m_src_model->dcmds()[index.row()];
// 			appendToFilters(dcmd);
// 		}
// 	}
}

void LogWidget::onRefillFilters ()
{
// 	if (m_tableview->model() == m_src_model || m_tableview->model() == m_proxy_model)
// 	{
// 		for (int i = 0, ie = m_src_model->dcmds().size(); i < ie; ++i)
// 		{
// 			DecodedCommand const & dcmd = m_src_model->dcmds()[i];
// 			appendToFilters(dcmd);
// 			appendToColorizers(dcmd);
// 		}
// 	}
// 	else //if (model() == m_find_proxy_model)
// 	{
// 		//refreshFilters(m_find_proxy_model);
// 	}
}


void LogWidget::clearFilters ()
{
	//@TODO: call all functions below
	//m_filter_state.clearFilters();
}

/*void LogWidget::appendToLvlWidgets (FilteredLevel const & flt)
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


/*void LogWidget::appendToCtxWidgets (FilteredContext const & flt)
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

}

