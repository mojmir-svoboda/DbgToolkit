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
	emit dockClosed();
	event->accept();
}

DockWidget * DockManager::mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name)
{
	return mkDockWidget(window, docked_widget, name, Qt::BottomDockWidgetArea);
}

DockWidget * DockManager::mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name, Qt::DockWidgetArea area)
{
	DockWidget * const dock = new DockWidget(name, window);
	//connect(dock, SIGNAL(dockClosed()), this, SLOT(onPlotClosed()));
	//docked_widget->setParent(dock);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	dock->setWidget(docked_widget);
	window->addDockWidget(area, dock);
	m_widgets.insert(name, dock);
	return dock;
}


