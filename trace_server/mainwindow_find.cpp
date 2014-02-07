#include "mainwindow.h"
#include <QApplication>

QWidget * getFocusedWidget ()
{
	return qApp->focusWidget();
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
