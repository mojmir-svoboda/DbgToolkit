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


struct DockedWidgetBase : ActionAble {

	DockedWidgetBase (QStringList const & path)
		: ActionAble(path)
		, m_wd(0) 
	{ }
	virtual ~DockedWidgetBase () { }

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

class DockTreeModel : public TreeModel<DockedInfo>
{
	Q_OBJECT
public:

	explicit DockTreeModel (QObject * parent = 0, tree_data_t * data = 0);
	~DockTreeModel ();

	QModelIndex insertItemWithPath (QStringList const & path, bool checked);

	TreeModel<DockedInfo>::node_t const * getItemFromIndex (QModelIndex const & index) const { return itemFromIndex(index); }
};


struct DockManager : QWidget, ActionAble
{
	Q_OBJECT
public:
	DockManager (MainWindow * mw, QStringList const & path);
	~DockManager ();

	typedef QMultiMap<QString, DockWidget *> widgets_t;
	typedef QMultiMap<QString, ActionAble *> actionables_t;
	widgets_t			m_widgets; // @TODO: hashed container?
	actionables_t		m_actionables;
	MainWindow * 		m_main_window;
	QDockWidget * 		m_docked_widgets;
	TreeView * 			m_docked_widgets_tree_view;
	DockTreeModel *		m_docked_widgets_model;
	typedef tree_filter<DockedInfo> data_filters_t;
	data_filters_t *	m_docked_widgets_data;

	DockWidget * mkDockWidget (DockedWidgetBase & dwb, bool visible);
	DockWidget * mkDockWidget (ActionAble & aa, bool visible, Qt::DockWidgetArea area);
	QModelIndex addDockedTreeItem (DockedWidgetBase & dwb, bool on);
	QModelIndex addActionTreeItem (ActionAble & aa, bool on);

	virtual bool handleAction (Action * a, E_ActionHandleType sync);

public slots:
	void onWidgetClosed (DockWidget * w);
	void onClickedAtDockedWidgets (QModelIndex idx);

};


#include <QStyledItemDelegate>
class CloseButton : public QStyledItemDelegate {
    Q_OBJECT
public:

    explicit CloseButton (QObject * parent = 0, QPixmap const & closeIcon = QPixmap());
    QPoint closeIconPos (QStyleOptionViewItem const & option) const;
    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    QSize sizeHint (QStyleOptionViewItem const & option, QModelIndex const & index) const;
    bool editorEvent (QEvent * event, QAbstractItemModel * model, QStyleOptionViewItem const & option, QModelIndex const & index);

signals:
    void closeIndexClicked(const QModelIndex &);

protected:
    QPixmap m_closeIcon;
    static const int margin = 2; // pixels to keep arount the icon

    Q_DISABLE_COPY(CloseButton)
};


