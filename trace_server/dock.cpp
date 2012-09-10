#include "dock.h"
#include <QString>
#include <QWidget>
#include <QDockWidget>
#include <QMultiMap>
#include <QCloseEvent>
#include <QMainWindow>

DockWidget::DockWidget (QString const & name, QMainWindow * const window)
	: QDockWidget(name, window)
{ }

void DockWidget::closeEvent (QCloseEvent * event)
{	
	emit dockClosed(this);
	event->accept();
}

DockWidget * DockManager::mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name)
{
	return mkDockWidget(window, docked_widget, name, Qt::BottomDockWidgetArea);
}

DockWidget * DockManager::mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name, Qt::DockWidgetArea area)
{
	DockWidget * const dock = new DockWidget(name, window);
	QObject::connect(dock, SIGNAL(dockClosed(DockWidget *)), this, SLOT(onPlotClosed(DockWidget *)));
	//docked_widget->setParent(dock);
	//dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	dock->setObjectName(name);
	dock->setWidget(docked_widget);
	window->addDockWidget(area, dock);
	m_widgets.insert(name, dock);
	return dock;
}

void DockManager::onPlotClosed (DockWidget * w)
{
	qDebug("%s w=%08x", __FUNCTION__, w);
	QObject::disconnect(w, SIGNAL(dockClosed(DockWidget *)), this, SLOT(onPlotClosed(DockWidget *)));
	m_widgets.remove(w->objectName());
	static_cast<QMainWindow *>(w->parent())->removeDockWidget(w);
	delete w;
	w = 0;
}

