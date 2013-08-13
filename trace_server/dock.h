#pragma once
#include <QString>
#include <QDockWidget>
#include <QMultiMap>
#include "treeview.h"
#include "treemodel.h"
#include "dockedwidget.h"
#include "action.h"
class QCloseEvent;
class QMainWindow;
class MainWindow;
struct DockManager;

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

class DockTreeModel : public TreeModel<DockedInfo>
{
	Q_OBJECT
public:

	explicit DockTreeModel (QObject * parent = 0, tree_data_t * data = 0);
	~DockTreeModel ();

	QModelIndex insertItemWithPath (QStringList const & path, bool checked);
};


struct DockManager : QObject
{
	Q_OBJECT
public:
	~DockManager ();

	QMultiMap<QStringList, QDockWidget *> m_widgets;
	MainWindow * 		m_main_window;
	DockWidget * 		m_docked_widgets;
	TreeView * 			m_docked_widgets_tree_view;
	DockTreeModel *		m_docked_widgets_model;
	typedef tree_filter<DockedInfo> data_filters_t;
	data_filters_t		m_docked_widgets_state;

	explicit DockManager (MainWindow * parent = 0);
	DockWidget * mkDockWidget (DockedWidgetBase & dwb, bool visible);
	DockWidget * mkDockWidget (DockedWidgetBase & dwb, bool visible, Qt::DockWidgetArea area);
	QModelIndex addDockedTreeItem (DockedWidgetBase & dwb, bool on);
	QModelIndex addTreeItem (QStringList const & path, bool on);

	void handleAction (Action * a);

public slots:
	void onWidgetClosed (DockWidget * w);

};


