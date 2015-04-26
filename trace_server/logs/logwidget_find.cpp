#include "logwidget.h"
#include <QStatusBar>
#include "logs/logtablemodel.h"
#include <filterproxymodel.h>
#include <logs/findproxymodel.h>
#include "utils.h"
#include <serialize.h>
#include "connection.h"
#include "mainwindow.h"
#include "warnimage.h"
#include <find_utils_table.h>

namespace logs {

void LogWidget::onFind ()
{
	//m_find_widget->onCancel();
	//w->setFocusProxy(m_find_widget);
	//m_find_widget->setFocusProxy(w); // dunno what the proxies are for
	//mk_action configure find widget
	FindConfig & cfg = m_config.m_find_config;
	m_find_widget->applyConfig(cfg);

	m_find_widget->onActivate();

	//setFocusProxy(m_find_widget);
	m_find_widget->setFocusProxy(this); // dunno what the proxies are for
}

void LogWidget::onFindNext ()
{
	m_find_widget->onFindNext();
}

void LogWidget::onFindPrev ()
{
	m_find_widget->onFindPrev();
}

void LogWidget::onFindAllRefs ()
{
	m_find_widget->onFindAllRefs();
}

void LogWidget::registerLinkedWidget (DockedWidgetBase * l)
{
	if (std::find(m_linked_widgets.begin(), m_linked_widgets.end(), l) == m_linked_widgets.end())
		m_linked_widgets.push_back(l);
}

void LogWidget::unregisterLinkedWidget (DockedWidgetBase * l)
{
	m_linked_widgets.erase(std::remove(m_linked_widgets.begin(), m_linked_widgets.end(), l), m_linked_widgets.end());
}

void LogWidget::linkToSource (LogWidget * src)
{
	m_linked_parent = src;
}

void LogWidget::setSrcModel (FindConfig const & fc)
{
	m_config.m_find_config = fc;
	m_tableview->setModel(m_src_model);
	m_src_model->resizeToCfg(m_config);
	m_find_proxy_selection = new QItemSelectionModel(m_find_proxy_model);
	m_tableview->setSelectionModel(m_src_selection);
	resizeSections();
	applyConfig();
}

void LogWidget::setFindProxyModel (FindConfig const & fc)
{
	m_config.m_find_config = fc;
	m_tableview->setModel(m_find_proxy_model);
	m_find_proxy_model->resizeToCfg(m_config);
	m_find_proxy_model->force_update();
	resizeSections();
	applyConfig();
}

LogWidget * LogWidget::mkFindAllRefsLogWidget (FindConfig const & fc)
{
	QString tag;
	if (fc.m_to_widgets.isEmpty())
		tag = "find_all_refs";
	else
	{
		// @TODO: validate widget form
		tag = fc.m_to_widgets.at(0);
	}

	LogConfig cfg;
	cfg.m_tag = tag;
	bool const loaded = m_connection->dataWidgetConfigPreload<e_data_log>(tag, cfg);
	if (!loaded)
	{
		cfg = m_config;	// inherit from parent widget if not loaded
	}
	cfg.m_tag = tag;
	cfg.m_show = true;
	cfg.m_filter_proxy = false;
	cfg.m_find_proxy = true;
	datalogs_t::iterator it = m_connection->dataWidgetFactoryFrom<e_data_log>(tag, cfg);

	LogWidget * child = *it;
	child->linkToSource(this);
	registerLinkedWidget(child);
	child->loadAuxConfigs();

	if (isModelProxy())
		child->setupRefsProxyModel(m_src_model, m_proxy_model); // @FIXME: could be any proxy, not just m_proxymodel
	else
		child->setupRefsModel(m_src_model);
	child->setFindProxyModel(fc);
	child->m_kfind_proxy_selection = new KLinkItemSelectionModel(child->m_tableview->model(), m_tableview->selectionModel());
	child->m_tableview->setSelectionModel(child->m_kfind_proxy_selection);
	child->applyConfig();
	child->m_config_ui.refreshUI();
	child->refreshFilters(child->m_find_proxy_model);

	child->setStyleSheet("\
			QHeaderView::section {\
			background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,\
											  stop:0 #616161, stop: 0.5 #505050,\
											  stop: 0.6 #434343, stop:1 #656565);\
			color: yellow;\
			padding-left: 4px;\
			border: 1px solid #6c6c6c;\
		}");

	//m_connection->getMainWindow()->onDockRestoreButton();
	return child;
}

LogWidget * LogWidget::mkFindAllCloneLogWidget (FindConfig const & fc)
{
	QString tag;
	if (fc.m_to_widgets.isEmpty())
	{
		if (fc.m_refs)
			tag = "find_all_refs";
		else if (fc.m_clone)
			tag = "find_all_clone";
		else
			tag = "noname";
	}
	else
	{
		// @TODO: validate widget form: appname/foo
		tag = fc.m_to_widgets.at(0);
	}

	LogConfig cfg;
	cfg.m_tag = tag;
	bool const loaded = m_connection->dataWidgetConfigPreload<e_data_log>(tag, cfg);
	if (!loaded)
	{
		cfg = m_config;	// inherit from parent widget if not loaded
	}
	cfg.m_tag = tag;
	cfg.m_show = true;
	cfg.m_filter_proxy = false;
	cfg.m_find_proxy = false;
	datalogs_t::iterator it = m_connection->dataWidgetFactoryFrom<e_data_log>(tag, cfg);

	LogWidget * child = *it;
	child->loadAuxConfigs();
	//child->applyConfig();
	LogTableModel * clone_model = cloneToNewModel(child, fc);
	child->setupCloneModel(clone_model);
	child->setSrcModel(fc);
	child->m_config_ui.refreshUI();

	return child;
}

void LogWidget::handleFindAction (FindConfig const & fc)
{
	bool const select_only = !fc.m_refs && !fc.m_clone;

	if (fc.m_regexp)
	{
		if (fc.m_regexp_val.pattern().isEmpty())
			return;
		if (!fc.m_regexp_val.isValid())
			return;
	}

	saveFindConfig();

	if (select_only)
	{
		if (fc.m_next)
			findAndSelectNext(m_tableview, fc);
		else if (fc.m_prev)
			findAndSelectPrev(m_tableview, fc);
		else
			findAndSelect(m_tableview, fc);
	}
	else
	{
		LogWidget * result_widget = 0;
		if (fc.m_refs)
		{
			result_widget = mkFindAllRefsLogWidget(fc);
		}
		else // clone
		{
			result_widget = mkFindAllCloneLogWidget(fc);
		}
	}
}

void LogWidget::showWarningSign ()
{
	qDebug("end of search");
	m_connection->getMainWindow()->statusBar()->showMessage(tr("End of document!"));

	// flash icon
	QPoint const global = rect().center();
	QPoint const pos(global.x() - m_warnimage->width() / 2, global.y() - m_warnimage->height() / 2);
    m_warnimage->move(pos);
	m_warnimage->show();
	m_warnimage->activateWindow();
	m_warnimage->raise();
	m_warnimage->warningFindNoMoreMatches();
}

QString LogWidget::findString4Tag (int tag, QModelIndex const & src_idx) const
{
	QString val;
	if (src_idx.isValid())
	{
		DecodedCommand const * dcmd = getDecodedCommand(src_idx);
		if (dcmd)
		{
			bool const exists = dcmd->getString(tag, val);
			if (!exists)
				qWarning("cannot find tag, src_row=%i %i", tag, src_idx.row());
		}
	}
	return val;
}
/*
QVariant LogWidget::findVariant4Tag (int tag, QModelIndex const & row_index) const
{
	int const idx = findColumn4TagCst(tag);
	if (idx == -1)
		return QVariant();

	LogTableModel * model = m_src_model;

	QModelIndex const model_idx = model->index(row_index.row(), idx, QModelIndex());
	if (model_idx.isValid())
	{
		QVariant value = model->data(model_idx);
		return value;
	}
	return QVariant();
}*/

}

