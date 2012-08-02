#pragma once
#include <QString>
#include <QWidget>
#include <QDockWidget>

inline QDockWidget * mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name)
{
	QDockWidget * const dock = new QDockWidget(name, window);
	docked_widget->setParent(dock);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	dock->setWidget(docked_widget);
	window->addDockWidget(Qt::TopDockWidgetArea, dock);
	return dock;
}

