#ifdef WIN32
#	define _WINSOCKAPI_ 
#endif
#include <QApplication>
#include <QClipboard>
#include <QtGui>
#include <QtNetwork/QtNetwork>
#include <stdlib.h>
#include "server.h"
#include "connection.h"
#include "mainwindow.h"
#include "utils.h"
#include "delegates.h"
#include "tableview.h"
#include "utils_qstandarditem.h"
#include <boost/function.hpp>

void Server::onClearCurrentView ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onClearCurrentView();
}
void Server::onClearCurrentFileFilter ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onClearCurrentFileFilter();
}
void Server::onClearCurrentCtxFilter ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onClearCurrentCtxFilter();
}
void Server::onClearCurrentTIDFilter ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onClearCurrentTIDFilter();
}
void Server::onClearCurrentRegexFilter ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onClearCurrentRegexFilter();
}
void Server::onClearCurrentStringFilter ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onClearCurrentStringFilter();
}
void Server::onClearCurrentColorizedRegexFilter ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onClearCurrentColorizedRegexFilter();
}
void Server::onClearCurrentScopeFilter ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onClearCurrentScopeFilter();
}
void Server::onClearCurrentRefTime ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onClearCurrentRefTime();
}

void Server::onHidePrevFromRow ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onHidePrevFromRow();
}

void Server::onUnhidePrevFromRow ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onUnhidePrevFromRow();
}

void Server::onExcludeFileLine ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onExcludeFileLine();
}

void Server::onToggleRefFromRow ()
{
	if (Connection * conn = findCurrentConnection())
		conn->onToggleRefFromRow();
}

void Server::onApplyColumnSetup ()
{
	qDebug("%s", __FUNCTION__);
	foreach (connections_t::value_type item, m_connections)
	{
		item.second->onApplyColumnSetup();
	}
}

template <typename T>
void Server::applyFnOnAllChildren (T fn, QAbstractItemModel * abs_model, Qt::CheckState state)
{
	QStandardItemModel * model = static_cast<QStandardItemModel *>(abs_model);
	QStandardItem * root = model->invisibleRootItem();
	QList<QStandardItem *> l = listChildren(root);

	if (Connection * conn = findCurrentConnection())
	{
		for (int i = 0, ie = l.size(); i < ie; ++i)
		{
			l.at(i)->setCheckState(state);
			QString const & data = model->data(l.at(i)->index(), Qt::DisplayRole).toString();
			fn(&conn->sessionState(), data);
		}
		conn->onInvalidateFilter();
	}
}

void Server::onSelectAllLevels ()
{
	boost::function<void (SessionState*, QString const &)> f = &SessionState::appendLvlFilter;
	applyFnOnAllChildren(f, m_main_window->getWidgetLvl()->model(), Qt::Checked);
}
void Server::onSelectNoLevels ()
{
	boost::function<void (SessionState*, QString)> f = &SessionState::removeLvlFilter;
	applyFnOnAllChildren(f, m_main_window->getWidgetLvl()->model(), Qt::Unchecked);
}

void Server::onSelectAllCtxs ()
{
	boost::function<void (SessionState*, QString)> f = &SessionState::appendCtxFilter;
	applyFnOnAllChildren(f, m_main_window->getWidgetCtx()->model(), Qt::Checked);
}

void Server::onSelectNoCtxs ()
{
	boost::function<void (SessionState*, QString)> f = &SessionState::removeCtxFilter;
	applyFnOnAllChildren(f, m_main_window->getWidgetCtx()->model(), Qt::Unchecked);
}

void Server::onClickedAtCtxTree (QModelIndex idx)
{
	QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetCtx()->model());
	QStandardItem * item = model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & ctx = model->data(idx, Qt::DisplayRole).toString();
	bool const orig_checked = (item->checkState() == Qt::Checked);
	if (Connection * conn = findCurrentConnection())
	{
		if (orig_checked)
			conn->sessionState().appendCtxFilter(ctx);
		else
			conn->sessionState().removeCtxFilter(ctx);
		conn->onInvalidateFilter();
	}
}

void Server::onDoubleClickedAtCtxTree (QModelIndex idx) { }

void Server::onClickedAtTIDList (QModelIndex idx)
{
	if (!idx.isValid())
		return;
	QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetTID()->model());
	QStandardItem * item = model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & val = model->data(idx, Qt::DisplayRole).toString();

	bool const orig_checked = (item->checkState() == Qt::Checked);
	if (Connection * conn = findCurrentConnection())
	{
		bool const checked = !orig_checked;
		if (checked)
			conn->sessionState().appendTIDFilter(val);
		else
			conn->sessionState().removeTIDFilter(val);
		conn->onInvalidateFilter();
	}
}

void Server::onClickedAtLvlList (QModelIndex idx)
{
	if (!idx.isValid())
		return;
	QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetLvl()->model());
	QStandardItem * item = model->itemFromIndex(idx);
	Q_ASSERT(item);

	bool checked = (item->checkState() == Qt::Checked);

	if (idx.column() == 1)
	{
		QString const & filter_item = model->data(model->index(idx.row(), 0, QModelIndex()), Qt::DisplayRole).toString();
		bool is_inclusive = true;
		QString const & mod = model->data(idx, Qt::DisplayRole).toString();

		E_LevelMode const curr = stringToLvlMod(mod.toStdString().c_str()[0]);
		size_t i = (curr + 1) % e_max_lvlmod_enum_value;
		E_LevelMode const act = static_cast<E_LevelMode>(i);
		//ui_settings->listViewColumnAlign->model()->setData(idx, QString(alignToString(act)));
		
		model->setData(idx, QString(lvlModToString(act)));

		if (mod == "I")
		{
			//model->setData(idx, QString("E"));
			is_inclusive = false;
		}
		else
		{
			//model->setData(idx, QString("I"));
		}

		if (Connection * conn = findCurrentConnection())
		{
			conn->sessionState().setLvlMode(filter_item, !checked, act);
			conn->onInvalidateFilter();
		}
	}
	else
	{
		QString const & filter_item = model->data(idx, Qt::DisplayRole).toString();
		if (Connection * conn = findCurrentConnection())
		{
			item->setCheckState(!checked ? Qt::Checked : Qt::Unchecked);
			if (!checked)
				conn->sessionState().appendLvlFilter(filter_item);
			else
				conn->sessionState().removeLvlFilter(filter_item);
			conn->onInvalidateFilter();
		}
	}
}
void Server::onDoubleClickedAtLvlList (QModelIndex idx)
{ }

void Server::onDoubleClickedAtTIDList (QModelIndex idx)
{ }

void Server::onClickedAtRegexList (QModelIndex idx)
{
	if (!idx.isValid())
		return;
	QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetRegex()->model());

	QString const & val = model->data(idx, Qt::DisplayRole).toString();

	if (idx.column() == 1)
	{
		QString const & reg = model->data(model->index(idx.row(), 0, QModelIndex()), Qt::DisplayRole).toString();
		std::string filter_item(reg.toStdString());
		bool is_inclusive = true;
		if (val == "I")
		{
			model->setData(idx, QString("E"));
			is_inclusive = false;
		}
		else
		{
			model->setData(idx, QString("I"));
		}

		QString const & val = model->data(idx, Qt::DisplayRole).toString();

		if (Connection * conn = findCurrentConnection())
		{
			conn->sessionState().setRegexInclusive(reg, is_inclusive);
			//conn->m_session_state.setRegexChecked(filter_item, checked);
			conn->recompileRegexps();
			conn->onInvalidateFilter();
		}
	}
	else
	{
		QString const & mod = model->data(model->index(idx.row(), 1, QModelIndex()), Qt::DisplayRole).toString();
		bool is_inclusive = false;
		if (mod == "I")
		{
			is_inclusive = true;
		}
		else
		{ }

		QStandardItem * item = model->itemFromIndex(idx);
		Q_ASSERT(item);
		bool const orig_checked = (item->checkState() == Qt::Checked);
		Qt::CheckState const checked = orig_checked ? Qt::Unchecked : Qt::Checked;
		item->setCheckState(checked);
		if (Connection * conn = findCurrentConnection())
		{
			// @TODO: if state really changed
			conn->sessionState().setRegexInclusive(val, is_inclusive);
			conn->m_session_state.setRegexChecked(val, checked);
			conn->recompileRegexps();
			conn->onInvalidateFilter();
		}

	}
}

void Server::onDoubleClickedAtRegexList (QModelIndex idx)
{ }

void Server::onClickedAtColorRegexList (QModelIndex idx)
{
	if (!idx.isValid()) return;
	QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetColorRegex()->model());
	QStandardItem * item = model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & val = model->data(idx, Qt::DisplayRole).toString();
	bool const orig_checked = (item->checkState() == Qt::Checked);
	Qt::CheckState const checked = orig_checked ? Qt::Unchecked : Qt::Checked;
	qDebug("color regex click! (checked=%u) %s ", checked, val.toStdString().c_str());
	item->setCheckState(checked);
	if (Connection * conn = findCurrentConnection())
	{
		// @TODO: if state really changed
		conn->m_session_state.setColorRegexChecked(val, checked);
		conn->recompileColorRegexps();
		conn->onInvalidateFilter();
	}
}

void Server::onDoubleClickedAtColorRegexList (QModelIndex idx)
{
}

void Server::onClickedAtStringList (QModelIndex idx)
{
	if (!idx.isValid())
		return;
	QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetString()->model());

	QString const & val = model->data(idx, Qt::DisplayRole).toString();

	if (idx.column() == 1)
	{
		QString const & reg = model->data(model->index(idx.row(), 0, QModelIndex()), Qt::DisplayRole).toString();
		std::string filter_item(reg.toStdString());
		bool is_inclusive = true;
		if (val == "I")
		{
			model->setData(idx, QString("E"));
			is_inclusive = false;
		}
		else
		{
			model->setData(idx, QString("I"));
		}

		QString const & val = model->data(idx, Qt::DisplayRole).toString();

		if (Connection * conn = findCurrentConnection())
		{
			conn->sessionState().setStringState(reg, is_inclusive);
			//conn->m_session_state.setRegexChecked(filter_item, checked);
			conn->recompileStrings();
		}
	}
	else
	{
		QString const & mod = model->data(model->index(idx.row(), 1, QModelIndex()), Qt::DisplayRole).toString();
		bool is_inclusive = false;
		if (mod == "I")
		{
			is_inclusive = true;
		}
		else
		{ }

		QStandardItem * item = model->itemFromIndex(idx);
		Q_ASSERT(item);
		bool const orig_checked = (item->checkState() == Qt::Checked);
		Qt::CheckState const checked = orig_checked ? Qt::Unchecked : Qt::Checked;
		item->setCheckState(checked);
		if (Connection * conn = findCurrentConnection())
		{
			// @TODO: if state really changed
			conn->sessionState().setStringState(val, is_inclusive);
			conn->m_session_state.setStringChecked(val, checked);
			conn->recompileStrings();
		}

	}
}

void Server::onDoubleClickedAtStringList (QModelIndex idx)
{ }

