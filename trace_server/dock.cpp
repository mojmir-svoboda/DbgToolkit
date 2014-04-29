#include "dock.h"
#include <QCloseEvent>
#include <QMainWindow>
#include "mainwindow.h"
#include <QApplication>

DockedWidgetBase::DockedWidgetBase (MainWindow * mw, QStringList const & path)
	: ActionAble(path)
	, m_main_window(mw)
	, m_dockwidget(0)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

DockedWidgetBase::~DockedWidgetBase ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	m_main_window->dockManager().removeActionAble(*this);
	m_dockwidget->setWidget(0);
	delete m_dockwidget;
	m_dockwidget = 0;
}


DockWidget::DockWidget (DockManager & mgr, QString const & name, QMainWindow * const window)
	: QDockWidget(name, window)
	, m_mgr(mgr)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);

    setStyleSheet(" \
		QDockWidget { color: black; border: 1px solid lightgray; } \
		QDockWidget::title { text-align: left; background: darkgray; padding-left: 5px; } \
		QDockWidget::close-button, QDockWidget::float-button { border: 1px solid transparent; background: darkgray; padding: 0px; } \
		QDockWidget::close-button:hover, QDockWidget::float-button:hover { background: gray; } \
		QDockWidget::close-button:pressed, QDockWidget::float-button:pressed { padding: 1px -1px -1px 1px; } \
		");
		//QDockWidget::title { text-align: left; background: #0068C6; padding-left: 5px; } \

}

DockWidget::~DockWidget ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	static_cast<QMainWindow *>(parent())->removeDockWidget(this);
}

void DockWidget::closeEvent (QCloseEvent * event)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	static_cast<QMainWindow *>(parent())->removeDockWidget(this);
	setWidget(0);
	m_mgr.onWidgetClosed(this);
	event->accept();
}

