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
	void dockClosed (DockWidget * w);
};


struct DockManager : public QObject
{
	Q_OBJECT
public:

	QMultiMap<QString, QDockWidget *> m_widgets;

	explicit DockManager (QObject * parent = 0) { }

	DockWidget * mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name);
	DockWidget * mkDockWidget (QMainWindow * const window, QWidget * const docked_widget, QString const & name, Qt::DockWidgetArea area);

public slots:
	void onPlotClosed (DockWidget * w);
};


