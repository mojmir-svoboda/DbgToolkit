#pragma once
#include <QString>
#include <QDockWidget>
#include <QMultiMap>
#include <QSpinBox>
#include "treeview.h"
#include "treemodel.h"
#include "dockedwidget.h"
#include "action.h"
#include "dockconfig.h"
#include "dockedconfig.h"
class QCloseEvent;
class QMainWindow;
class MainWindow;
struct DockManager;

class DockTreeView : public TreeView
{
	Q_OBJECT
public:

	DockTreeView (QWidget * parent = 0);

protected:
};


struct DockedWidgetBase : ActionAble {

	DockedWidgetBase (QStringList const & path)
		: ActionAble(path)
		, m_wd(0) 
	{ }
	virtual ~DockedWidgetBase () { }

	virtual DockedConfigBase const & dockedConfig () const = 0;
	virtual DockedConfigBase & dockedConfig () = 0;

	QDockWidget * m_wd;

	virtual E_DataWidgetType type () const = 0;
};



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

enum {
	e_DockRoleCentralWidget = Qt::UserRole + 1,
	e_DockRoleSelect,
	e_DockRoleSyncGroup
};

class DockTreeModel : public TreeModel<DockedInfo>
{
	Q_OBJECT
public:

	explicit DockTreeModel (QObject * parent = 0, tree_data_t * data = 0);
	~DockTreeModel ();

	QModelIndex insertItemWithPath (QStringList const & path, bool checked);

	TreeModel<DockedInfo>::node_t const * getItemFromIndex (QModelIndex const & index) const { return itemFromIndex(index); }

	virtual int columnCount (QModelIndex const & parent) const;
	virtual QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags (QModelIndex const & index) const;
	virtual bool setData (QModelIndex const & index, QVariant const & value, int role = Qt::EditRole);

public slots:
	void onExpanded (QModelIndex const & idx) { expanded(idx); }
	void onCollapsed (QModelIndex const & idx) { collapsed(idx); }
};


struct DockManager : DockTreeView, ActionAble
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
	DockTreeModel *		m_docked_widgets_model;
	typedef tree_filter<DockedInfo> data_filters_t;
	data_filters_t *	m_docked_widgets_data;
	DockConfig			m_config;
	DockConfig			m_config2;

	DockWidget * mkDockWidget (DockedWidgetBase & dwb, bool visible);
	DockWidget * mkDockWidget (ActionAble & aa, bool visible, Qt::DockWidgetArea area);
	QModelIndex addDockedTreeItem (DockedWidgetBase & dwb, bool on);
	QModelIndex addActionTreeItem (ActionAble & aa, bool on);
	DockedWidgetBase * findDockable (QString const & joined_path);

	void loadConfig (QString const & path);
	void saveConfig (QString const & path);

	virtual bool handleAction (Action * a, E_ActionHandleType sync);

public slots:
	void onWidgetClosed (DockWidget * w);
	void onClickedAtDockedWidgets (QModelIndex idx);

protected slots:
	void onColumnResized (int column, int oldSize, int newSize);

};


#include <QStyledItemDelegate>
class DockedTreeDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:

    explicit DockedTreeDelegate (QObject * parent = 0, QPixmap const & icon = QPixmap());
    QPoint calcIconPos (QStyleOptionViewItem const & option) const;
    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    QSize sizeHint (QStyleOptionViewItem const & option, QModelIndex const & index) const;
    bool editorEvent (QEvent * event, QAbstractItemModel * model, QStyleOptionViewItem const & option, QModelIndex const & index);

signals:
    void closeIndexClicked (QModelIndex const &);

protected:
    QPixmap m_icon;
    static const int margin = 2; // pixels to keep arount the icon

    Q_DISABLE_COPY(DockedTreeDelegate)
};


class SpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    SpinBoxDelegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    void setEditorData(QWidget *editor, QModelIndex const &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, QModelIndex const &index) const;
    void updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const &option, QModelIndex const &index) const;
};


