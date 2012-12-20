#include "dock.h"
#include <QString>
#include <QWidget>
#include <QDockWidget>
#include <QMultiMap>
#include <QCloseEvent>
#include <QMainWindow>

DockWidget::DockWidget (DockManager & mgr, QString const & name, QMainWindow * const window)
	: QDockWidget(name, window)
	, m_mgr(mgr)
{ }

void DockWidget::closeEvent (QCloseEvent * event)
{	
	static_cast<QMainWindow *>(parent())->removeDockWidget(this);
	setWidget(0);
	m_mgr.onPlotClosed(this);
	event->accept();
}

DockWidget * DockManager::mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name)
{
	return mkDockWidget(window, docked_widget, name, Qt::BottomDockWidgetArea);
}

DockWidget * DockManager::mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name, Qt::DockWidgetArea area)
{
	DockWidget * const dock = new DockWidget(*this, name, window);
	dock->setObjectName(name);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	dock->setWidget(docked_widget);
	window->addDockWidget(area, dock);
	m_widgets.insert(name, dock);
	dock->setAttribute(Qt::WA_DeleteOnClose, false);
	return dock;
}

void DockManager::onPlotClosed (DockWidget * w)
{
	qDebug("%s w=%08x", __FUNCTION__, w);
	m_widgets.remove(w->objectName());
}

