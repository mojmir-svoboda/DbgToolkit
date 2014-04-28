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
#include "controlbarcommon.h"
class QCloseEvent;
class MainWindow;
struct DockWidget;
class ControlBarCommon;
namespace Ui { class ControlBarCommon; }

struct DockManager : DockManagerView, ActionAble
{
	Q_OBJECT
public:

	DockManager (MainWindow * mw, QStringList const & path);
	~DockManager ();

	enum {
		e_Column_Name,
		e_Column_Close,
		e_Column_ControlWidget,
		e_max_dockmgr_column
	};

	typedef QMultiMap<QString, DockWidget *> widgets_t;
	typedef QMultiMap<QString, ActionAble *> actionables_t;
	widgets_t			m_widgets; // @TODO: hashed container?
	actionables_t		m_actionables;
	MainWindow * 		m_main_window;
	QDockWidget * 		m_docked_widgets;
	ControlBarCommon * m_control_bar;
	DockManagerModel *	m_model;
	typedef tree_filter<DockedInfo> data_t;
	data_t *	        m_model_data;
	DockManagerConfig	m_config;

	DockWidget * mkDockWidget (DockedWidgetBase & dwb, bool visible);
	DockWidget * mkDockWidget (ActionAble & aa, bool visible, Qt::DockWidgetArea area);

	QModelIndex addActionAble (ActionAble & aa, bool on);
	void removeActionAble (ActionAble & aa);
	ActionAble const * findActionAble (QString const & dst_joined) const;
	Ui::ControlBarCommon * controlUI () { return m_control_bar->ui; }
	Ui::ControlBarCommon const * controlUI () const { return m_control_bar->ui; }

	void loadConfig (QString const & path);
	void saveConfig (QString const & path);
	void applyConfig ();

	virtual bool handleAction (Action * a, E_ActionHandleType sync);
	virtual QWidget * controlWidget () { return m_control_bar; }

public slots:
	void onWidgetClosed (DockWidget * w);
	void onClicked (QModelIndex idx);

protected slots:
	void onColumnResized (int column, int oldSize, int newSize);

};



