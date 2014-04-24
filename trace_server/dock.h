#pragma once
#include <QDockWidget>
#include "action.h"
#include "types.h"
#include "dockedconfig.h"

struct DockedWidgetBase : QObject, ActionAble {
	Q_OBJECT
public:
	DockedWidgetBase (QStringList const & path)
		: ActionAble(path)
		, m_dockwidget(0)
	{ }
	virtual ~DockedWidgetBase () { }

	//virtual DockedConfigBase const & dockedConfig () const = 0;
	//virtual DockedConfigBase & dockedConfig () = 0;
	//virtual QWidget * dockedWidget () = 0;
	virtual QWidget * controlWidget () = 0;

	QDockWidget * m_dockwidget;

	virtual E_DataWidgetType type () const = 0;
};


class QCloseEvent; class QMainWindow; struct DockManager;

struct DockWidget : public QDockWidget
{
	Q_OBJECT
	friend struct DockManager;
public:

	explicit DockWidget (DockManager & mgr, QString const & name, QMainWindow * const window);
	virtual void closeEvent (QCloseEvent * event);

Q_SIGNALS:
	void dockClosed (DockWidget * w);

private:

	DockManager & m_mgr;
};

