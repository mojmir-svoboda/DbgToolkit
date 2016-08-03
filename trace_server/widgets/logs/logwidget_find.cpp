#include "logwidget.h"
#include <QStatusBar>
#include "logtablemodel.h"
#include "logfilterproxymodel.h"
#include "findproxymodel.h"
#include <utils/utils.h>
#include <serialize/serialize.h>
#include "connection.h"
#include "mainwindow.h"
#include <widgets/warnimage.h>
#include <utils/find_utils_table.h>


template<class L>
struct do_recurse_impl;

template<template<class...> class L>
struct do_recurse_impl<L<>>
{
	template<class FnT>
	static void apply (FnT fn) { }
};

template<template<class...> class L, class FirstT, class... RestT>
struct do_recurse_impl<L<FirstT, RestT...>>
{
	template<class FnT>
	static void apply (FnT fn)
	{
		fn(FirstT::value);
		do_recurse_impl<L<RestT...>>::template apply<FnT>(fn);
	}
};

template<class L, class FnT>
void recurse (FnT fn)
{
	do_recurse_impl<L>::template apply<FnT>(fn);
}



namespace logs {


void fillDefaultConfigWithLogTags (CheckedComboBoxConfig & cccfg)
{
	if (!cccfg.hasValidConfig())
	{
		cccfg.clear();

		cccfg.m_base.reserve(proto::e_max_tag_count);
		cccfg.m_states.reserve(proto::e_max_tag_count);
		
		recurse<proto::log_format>(
			[&cccfg] (unsigned tag)
			{
				cccfg.m_base.push_back(proto::get_tag_name(tag));
				cccfg.m_states.push_back(false);
			});

		cccfg.m_states[proto::tag2col<proto::int_<proto::tag_msg>>::value] = true;
		//cccfg.m_current = proto::get_tag_name(proto::tag2col<proto::int_<proto::tag_msg>>::value);
		cccfg.m_current = "custom";
		cccfg.m_combined_states.push_back("custom");
	}
}

void LogWidget::onFind ()
{
	//m_find_widget->onCancel();
	//w->setFocusProxy(m_find_widget);
	//m_find_widget->setFocusProxy(w); // dunno what the proxies are for
	//mk_action configure find widget
	FindConfig & cfg = m_config.m_find_config;

	if (!cfg.m_where.hasValidConfig())
		fillDefaultConfigWithLogTags(cfg.m_where);

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
/*	m_src_model->resizeToCfg(m_config);*/
	m_find_proxy_selection = new QItemSelectionModel(m_find_proxy_model);
	m_tableview->setSelectionModel(m_src_selection);
	applyConfig();
}

void LogWidget::setFindProxyModel (FindConfig const & fc)
{
	m_config.m_find_config = fc;
	m_tableview->setModel(m_find_proxy_model);
	m_src_model->addProxy(m_find_proxy_model);
/*	m_find_proxy_model->resizeToCfg(m_config);*/
	m_find_proxy_model->force_update();
	applyConfig();
}

LogWidget * LogWidget::mkFindAllRefsLogWidget (FindConfig const & fc)
{
	QString tag;
	//if (fc.m_to_widgets.isEmpty())
		tag = "find_all_refs";
	//else
	{
		// @TODO: validate widget form
		//tag = fc.m_to_widgets.at(0);
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
	child->m_config_ui->refreshUI();
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
	//if (fc.m_to_widgets.isEmpty())
	{
		if (fc.m_refs)
			tag = "find_all_refs";
		else if (fc.m_clone)
			tag = "find_all_clone";
		else
			tag = "noname";
	}
// 	else
// 	{
// 		// @TODO: validate widget form: appname/foo
// 		tag = fc.m_to_widgets.at(0);
// 	}

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
	child->m_config_ui->refreshUI();

	return child;
}

void LogWidget::findAndSelect (FindConfig const & fc)
{
	QModelIndex start = m_tableview->model()->index(0, 0);

	// if selected column
	//	QModelIndexList indexes = model()->match(start, Qt::DisplayRole, QVariant(fc.m_str), -1, Qt::MatchFixedString);
	QModelIndexList indexes;
	findInWholeTable(m_tableview, fc, indexes);

	QItemSelectionModel * selection_model = m_tableview->selectionModel();

	QItemSelection selection;
	foreach(QModelIndex index, indexes)
	{

		QModelIndex left = m_tableview->model()->index(index.row(), 0);
		QModelIndex right = m_tableview->model()->index(index.row(), m_tableview->model()->columnCount() - 1);

		QItemSelection sel(left, right);
		selection.merge(sel, QItemSelectionModel::Select);
	}
	selection_model->select(selection, QItemSelectionModel::Select);

	m_connection->getMainWindow()->onStatusChanged(tr("Selected %1 lines").arg(selection.size()));
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
			findAndSelect(fc);
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

void LogWidget::onQuickString ()
{
	QuickStringConfig & cfg = m_config.m_quick_string_config;

	if (!cfg.m_where.hasValidConfig())
		fillDefaultConfigWithLogTags(cfg.m_where);

	m_quick_string_widget->applyConfig(cfg);
	m_quick_string_widget->onActivate();
}

}

