#pragma once
#include <QString>
#include <QDockWidget>
#include <QMultiMap>
class QCloseEvent;
class QMainWindow;

struct DockWidget : public QDockWidget
{
	Q_OBJECT
public:

	explicit DockWidget (QString const & name, QMainWindow * const window);
	virtual void closeEvent (QCloseEvent * event);

Q_SIGNALS:
	void dockClosed ();
};


struct DockManager
{

	QMultiMap<QString, QDockWidget *> m_widgets;

	DockManager () { }

	DockWidget * mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name);
	DockWidget * mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name, Qt::DockWidgetArea area);
};


