#include "mainwindow.h"
#include <QApplication>
#include "widgets/quickstringconfig.h"
#include "widgets/quickstringwidget.h"

QWidget * getFocusedWidget ()
{
	return qApp->focusWidget();
}

void MainWindow::onAllFind ()
{
	QuickStringConfig & cfg = m_config.m_quick_string_config;

	Action a;
	a.m_type = e_Find;
	a.m_src_path = dockManager().path();
	a.m_src = &m_dock_mgr;

	for (auto it = m_dock_mgr.m_actionables.begin(), ite = m_dock_mgr.m_actionables.end(); it != ite; ++it)
		(*it)->handleAction(&a, e_Sync);
}


void MainWindow::onAllPopAction ()
{
	Action a;
	a.m_type = e_Pop;
	a.m_src_path = dockManager().path();
	a.m_src = &m_dock_mgr;

	for (auto it = m_dock_mgr.m_actionables.begin(), ite = m_dock_mgr.m_actionables.end(); it != ite; ++it)
		(*it)->handleAction(&a, e_Sync);
}

void MainWindow::onAllQuickString ()
{
	QuickStringConfig & cfg = m_config.m_quick_string_config;

//	if (!cfg.m_where.hasValidConfig())
//		fillDefaultConfigWithLogTags(cfg.m_where);

	m_quick_string_widget->applyConfig(cfg);
	m_quick_string_widget->onActivate();
}

/*void MainWindow::handleFindVisibility()
{
	QWidget * w = getFocusedWidget();

	if (!w)
		return;

	if (w == m_find_widget || w->parent() == m_find_widget)
		return;

	if (m_find_widget)
	{
		m_find_widget->onCancel();

		if (DockedWidgetBase * dwb = m_dock_mgr.findDockableForWidget(w))
		{
			m_find_widget->setParent(w);
			//w->setFocusProxy(m_find_widget);
			//m_find_widget->setFocusProxy(w); // dunno what the proxies are for
			//mk_action configure find widget
			FindConfig & cfg = dwb->dockedConfig().m_find_config;
			m_find_widget->applyConfig(cfg);
			m_find_widget->setDockedWidget(dwb);
		}
	}

	if (m_find_widget)
	{
		m_find_widget->onActivate();
	}
}*/
/*
void MainWindow::onFind ()
{
	handleFindVisibility();
}

void MainWindow::onFindNext ()
{
	handleFindVisibility();
	m_find_widget->onFindNext();
}

void MainWindow::onFindPrev ()
{
	handleFindVisibility();
	m_find_widget->onFindPrev();
}

void MainWindow::onFindAllRefs ()
{
	handleFindVisibility();
	m_find_widget->onFindAllRefs();
}

*/
