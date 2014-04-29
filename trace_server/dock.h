#pragma once
#include <QDockWidget>
#include "action.h"
#include "types.h"
#include "dockedconfig.h"

struct DockWidget : public QDockWidget
{
	Q_OBJECT
	friend struct DockManager;
public:

	explicit DockWidget (DockManager & mgr, QString const & name, QMainWindow * const window);
	virtual ~DockWidget ();
	virtual void closeEvent (QCloseEvent * event);

	void hideEvent (QHideEvent *) { emit widgetVisibilityChanged(false); }
	void showEvent (QShowEvent *) { emit widgetVisibilityChanged(true); }

signals:
	void dockClosed (DockWidget * w);
	void widgetVisibilityChanged (bool state);

private:
	DockManager & m_mgr;
};


class QCloseEvent; class QMainWindow; struct DockManager; class MainWindow;

struct DockedWidgetBase : ActionAble {
	//Q_OBJECT
public:
	MainWindow * m_main_window;
	DockWidget * m_dockwidget;

	DockedWidgetBase (MainWindow * mw, QStringList const & path);
	virtual ~DockedWidgetBase ();

	virtual E_DataWidgetType type () const = 0;
	virtual QWidget * controlWidget () = 0;

	void setDockWidget (DockWidget * w) { m_dockwidget = w; }
	DockWidget * dockWidget () { return m_dockwidget; }
	DockWidget const * dockWidget () const { return m_dockwidget; }
};


