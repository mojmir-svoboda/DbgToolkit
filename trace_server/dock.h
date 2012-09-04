#pragma once
#include <QString>
#include <QWidget>
#include <QDockWidget>
#include <QMultiMap>

struct DockManager {

	QMultiMap<QString, QDockWidget *> m_widgets;

	DockManager () { }
	

	QDockWidget * mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name)
	{
		return mkDockWidget(window, docked_widget, name, Qt::BottomDockWidgetArea);
	}

	QDockWidget * mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name, Qt::DockWidgetArea area)
	{
		QDockWidget * const dock = new QDockWidget(name, window);
		//docked_widget->setParent(dock);
		dock->setAllowedAreas(Qt::AllDockWidgetAreas);
		dock->setWidget(docked_widget);
		window->addDockWidget(area, dock);
		m_widgets.insert(name, dock);
		return dock;
	}


};


