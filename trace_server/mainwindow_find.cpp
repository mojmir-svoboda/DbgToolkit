#include "mainwindow.h"
#include <QApplication>

QWidget * getFocusedWidget ()
{
	return qApp->focusWidget();
}

void MainWindow::handleFindVisibility()
{
	if (m_find_widget)
	{
		m_find_widget->onCancel();
	}

	QWidget * w = getFocusedWidget();
	if (DockedWidgetBase * dwb = m_dock_mgr.findDockableForWidget(w))
	{
		m_find_widget->setParent(w);
		//mk_action configure find widget
		FindConfig & cfg = dwb->dockedConfig().m_find_config;
		m_find_widget->applyConfig(cfg);
		m_find_widget->m_dwb = dwb;
	}

	if (m_find_widget)
	{
		m_find_widget->onActivate();
	}
}

//void MainWindow::handleFindVisibility()

void MainWindow::onFind ()
{
	handleFindVisibility();
}

void MainWindow::onFindNext ()
{
	handleFindVisibility();
}

void MainWindow::onFindPrev ()
{
	handleFindVisibility();
}

void MainWindow::onFindAllRefs ()
{
	handleFindVisibility();
}
