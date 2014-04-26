#include "dock.h"
#include <QCloseEvent>
#include <QMainWindow>
#include "mainwindow.h"

DockedWidgetBase::DockedWidgetBase (QStringList const & path)
	: ActionAble(path)
	, m_dockwidget(0)
{ }

DockWidget::DockWidget (DockManager & mgr, QString const & name, QMainWindow * const window)
	: QDockWidget(name, window)
	, m_mgr(mgr)
{ }

void DockWidget::closeEvent (QCloseEvent * event)
{
	qDebug("%s", __FUNCTION__);
	static_cast<QMainWindow *>(parent())->removeDockWidget(this);
	setWidget(0);
	m_mgr.onWidgetClosed(this);
	event->accept();
}

