#pragma once
#include <QString>
#include <QDockWidget>
#include <QMultiMap>
#include <QSpinBox>
#include "action.h"
#include "dockedconfig.h"
#include "dockmanagerview.h"
#include "dockmanagermodel.h"
#include "dockmanagerconfig.h"
#include "controlbar_dockmanager.h"
class QCloseEvent;
class MainWindow;
struct DockWidget;
class ControlBarDockManager;
namespace Ui { class ControlBarDockManager; }

enum {
	e_DockRoleCentralWidget = Qt::UserRole + 1,
	e_DockRoleSelect,
	e_DockRoleSyncGroup
};

struct DockManager : DockManagerView, ActionAble
{
	Q_OBJECT
public:
	
	DockManager (MainWindow * mw, QStringList const & path);
	~DockManager ();

	typedef QMultiMap<QString, DockWidget *> widgets_t;
	typedef QMultiMap<QString, ActionAble *> actionables_t;
	typedef QMultiMap<QString, DockedWidgetBase *> dockables_t;
	widgets_t			m_widgets; // @TODO: hashed container?
	dockables_t			m_dockables;
	actionables_t		m_actionables;
	MainWindow * 		m_main_window;
	QDockWidget * 		m_docked_widgets;
	ControlBarDockManager * m_control_bar;
	DockManagerModel *	m_model;
	typedef tree_filter<DockedInfo> data_t;
	data_t *	        m_model_data;
	DockManagerConfig	m_config;

	DockWidget * mkDockWidget (DockedWidgetBase & dwb, bool visible);
	DockWidget * mkDockWidget (ActionAble & aa, bool visible, Qt::DockWidgetArea area);
	QModelIndex addDockedTreeItem (DockedWidgetBase & dwb, bool on);
	QModelIndex addActionTreeItem (ActionAble & aa, bool on);
	DockedWidgetBase const * findDockable (QString const & joined_path) const;

    void removeDockable (QString const & dst_joined);
    void removeActionAble (QString const & dst_joined);
	DockedWidgetBase * findDockable (QString const & joined_path);
	ActionAble const * findActionAble (QString const & dst_joined) const;
	DockedWidgetBase const * findDockableForWidget (QWidget * w) const;
	DockedWidgetBase * findDockableForWidget (QWidget * w);
	Ui::ControlBarDockManager * controlUI () { return m_control_bar->ui; }
	Ui::ControlBarDockManager const * controlUI () const { return m_control_bar->ui; }

	void loadConfig (QString const & path);
	void saveConfig (QString const & path);
	void applyConfig ();

	virtual bool handleAction (Action * a, E_ActionHandleType sync);

public slots:
	void onWidgetClosed (DockWidget * w);
	void onClicked (QModelIndex idx);

protected slots:
	void onColumnResized (int column, int oldSize, int newSize);

};



