#include "dock.h"
#include <QString>
#include <QWidget>
#include <QDockWidget>
#include <QMultiMap>
#include <QCloseEvent>
#include <QMainWindow>
#include "mainwindow.h"
#include "connection.h"

#include <QStyledItemDelegate>
    DockedTreeDelegate::DockedTreeDelegate (QObject * parent, QPixmap const & icon)
        : QStyledItemDelegate(parent)
        , m_icon(icon)
    {
        if (m_icon.isNull())
        {
            m_icon = qApp->style()->standardPixmap(QStyle::SP_DialogCloseButton);
        }
    }

    QPoint DockedTreeDelegate::calcIconPos (QStyleOptionViewItem const & option) const
	{
        return QPoint(option.rect.right() - m_icon.width() - margin,
                      option.rect.center().y() - m_icon.height()/2);
    }

    void DockedTreeDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
	{
		TreeView * t = static_cast<TreeView *>(parent());
		DockTreeModel * m = static_cast<DockTreeModel *>(t->model());

		TreeModel<DockedInfo>::node_t const * n = m->getItemFromIndex(index);

		int const col = index.column();
		if (col == e_InCentralWidget)
		{
			if (n->data.m_centralwidget)
				painter->drawPixmap(calcIconPos(option), m_icon);
		}
        QStyledItemDelegate::paint(painter, option, index);
        // Only display the close icon for top level items...
        //if(!index.parent().isValid()
                // ...and when the mouse is hovering the item
                // (mouseTracking must be enabled on the view)
                //&& (option.state & QStyle::State_MouseOver))
				//)
    }

    QSize DockedTreeDelegate::sizeHint (QStyleOptionViewItem const & option, QModelIndex const & index) const
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);

        // Make some room for the close icon
        if (!index.parent().isValid()) {
            size.rwidth() += m_icon.width() + margin * 2;
            size.setHeight(qMax(size.height(), m_icon.height() + margin * 2));
        }
        return size;
    }

    bool DockedTreeDelegate::editorEvent (QEvent * event, QAbstractItemModel * model, QStyleOptionViewItem const & option, QModelIndex const & index)
    {
        // Emit a signal when the icon is clicked
        if (!index.parent().isValid() && event->type() == QEvent::MouseButtonRelease)
		{
            QMouseEvent const * mouseEvent = static_cast<QMouseEvent const *>(event);
            QRect const closeButtonRect = m_icon.rect().translated(calcIconPos(option));
            if (closeButtonRect.contains(mouseEvent->pos()))
            {
                emit closeIndexClicked(index);
            }
        }
        return false;
    }


DockWidget::DockWidget (DockManager & mgr, QString const & name, QMainWindow * const window)
	: QDockWidget(name, window)
	, m_mgr(mgr)
{ }

void DockWidget::closeEvent (QCloseEvent * event)
{	
	static_cast<QMainWindow *>(parent())->removeDockWidget(this);
	setWidget(0);
	m_mgr.onWidgetClosed(this);
	event->accept();
}

DockManager::DockManager (MainWindow * mw, QStringList const & path)
	: QWidget(mw), ActionAble(path)
	, m_main_window(mw)
	, m_docked_widgets(0)
	, m_docked_widgets_tree_view(0)
	, m_docked_widgets_model(0)
	, m_docked_widgets_data(0)
{
	m_docked_widgets_data = new data_filters_t();
	m_docked_widgets_model = new DockTreeModel(this, m_docked_widgets_data);
	m_docked_widgets_tree_view = new TreeView(this);
	m_docked_widgets_tree_view->setModel(m_docked_widgets_model);

	QPixmap icons_for_cols[e_max_action_type];
	icons_for_cols[e_Visibility] = QPixmap();
	icons_for_cols[e_InCentralWidget] = qApp->style()->standardPixmap(QStyle::SP_DesktopIcon);
	icons_for_cols[e_SyncGroup] = QPixmap();
	icons_for_cols[e_Select] = qApp->style()->standardPixmap(QStyle::SP_DialogCloseButton);
	icons_for_cols[e_AlignH] = qApp->style()->standardPixmap(QStyle::SP_ToolBarHorizontalExtensionButton);
	icons_for_cols[e_AlignV] = qApp->style()->standardPixmap(QStyle::SP_ToolBarVerticalExtensionButton);

	for (int i = e_InCentralWidget; i < e_max_action_type; ++i)
	{
		m_docked_widgets_tree_view->setItemDelegateForColumn(i, new DockedTreeDelegate(m_docked_widgets_tree_view, icons_for_cols[i]));
	}

	QString const name = path.join("/");
	QDockWidget * const dock = new QDockWidget(this);
	dock->setObjectName(name);
	dock->setWindowTitle(name);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_main_window->addDockWidget(Qt::BottomDockWidgetArea, dock);
	m_actionables.insert(name, this);
	dock->setAttribute(Qt::WA_DeleteOnClose, false);
	dock->setWidget(m_docked_widgets_tree_view);

	//if (visible) 
	//	m_main_window->restoreDockWidget(dock);
	m_docked_widgets = dock;

	connect(m_docked_widgets_tree_view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtDockedWidgets(QModelIndex)));
	connect(m_docked_widgets, SIGNAL(visibilityChanged(bool)), this, SLOT(onListVisibilityChanged(bool)));
	connect(m_docked_widgets, SIGNAL(dockClosed()), mw, SLOT(onDockManagerClosed()));
}

DockManager::~DockManager ()
{
	disconnect(m_docked_widgets_tree_view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtDockedWidgets(QModelIndex)));
}

DockWidget * DockManager::mkDockWidget (DockedWidgetBase & dwb, bool visible)
{
	return mkDockWidget(dwb, visible, Qt::BottomDockWidgetArea);
}

DockWidget * DockManager::mkDockWidget (ActionAble & aa, bool visible, Qt::DockWidgetArea area)
{
	Q_ASSERT(aa.path().size() > 0);

	QString const name = aa.path().join("/");
	DockWidget * const dock = new DockWidget(*this, name, m_main_window);
	dock->setObjectName(name);
	dock->setWindowTitle(name);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	//dock->setWidget(docked_widget); // set by caller
	m_main_window->addDockWidget(area, dock);
	m_widgets.insert(name, dock);
	dock->setAttribute(Qt::WA_DeleteOnClose, false);

	if (visible) 
		m_main_window->restoreDockWidget(dock);
	//static_cast<MainWindow *>(window)->onDockedWidgetsToolButton();
	return dock;
}

void DockManager::onWidgetClosed (DockWidget * w)
{
	qDebug("%s w=%08x", __FUNCTION__, w);
	m_widgets.remove(w->objectName());
}

/*void MainWindow::onListVisibilityChanged (bool visible)
{
	ui->dockedWidgetsToolButton->setChecked(visible);
}*/


QModelIndex DockManager::addDockedTreeItem (DockedWidgetBase & dwb, bool on)
{
	return addActionTreeItem(dwb, on);
}


QModelIndex DockManager::addActionTreeItem (ActionAble & aa, bool on)
{
	QModelIndex const idx = m_docked_widgets_model->insertItemWithPath(aa.path(), on);
	// @TODO: set type=AA into returned node
	m_docked_widgets_model->setData(idx, QVariant(on ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
	QString const & name = aa.joinedPath();
	m_actionables.insert(name, &aa);
	aa.m_idx = idx;
	return idx;
}

bool DockManager::handleAction (Action * a, E_ActionHandleType sync)
{
	QStringList const & src_path = a->m_src_path;
	QStringList const & dst_path = a->m_dst_path;
	QStringList const & my_addr = path();

	if (dst_path.size() == 0)
	{
		qWarning("DockManager::handleAction empty dst");
		return false;
	}

	Q_ASSERT(my_addr.size() == 1);
	int const lvl = dst_path.indexOf(my_addr.at(0), a->m_dst_curr_level);
	if (lvl == -1)
	{
		qWarning("DockManager::handleAction message not for me");
		return false;
	}
	else if (lvl == dst_path.size() - 1)
	{
		// message just for me! gr8!
		a->m_dst_curr_level = lvl;
		a->m_dst = this;
		return true;
	}
	else
	{
		// message for my children
		a->m_dst_curr_level = lvl + 1;

		if (a->m_dst_curr_level >= 0 && a->m_dst_curr_level < dst_path.size())
		{
			QString const dst_joined = dst_path.join("/");
			actionables_t::iterator it = m_actionables.find(dst_joined);
			if (it != m_actionables.end() && it.key() == dst_joined)
			{
				ActionAble * const next_hop =  it.value();
				next_hop->handleAction(a, sync);
				++it;
			}
			else
			{
				//@TODO: find least common subpath
			}
		}
		else
		{
			qWarning("DockManager::handleAction hmm? what?");
		}
	}

	return false;
}

void DockManager::onClickedAtDockedWidgets (QModelIndex idx)
{
	TreeModel<DockedInfo>::node_t const * n = m_docked_widgets_model->getItemFromIndex(idx);
	QStringList const & dst = n->data.m_path;

	int const col = idx.column();
	Action a;
	a.m_type = static_cast<E_ActionType>(col);
	a.m_src_path = path();
	a.m_src = this;
	a.m_dst_path = dst;
	if (col == e_Visibility)
	{
		int const state = m_docked_widgets_model->data(idx, Qt::CheckStateRole).toInt();
		a.m_args.push_back(state);
	}
	if (col == e_InCentralWidget)
	{
		int const state = n->data.m_centralwidget;
		int const new_state = state == 0 ? 1 : 0;

		m_docked_widgets_model->setData(idx, new_state, e_DockRoleCentralWidget);
		a.m_args.push_back(new_state);
	}

	//av.m_dst = 0;
	handleAction(&a, e_Sync);
}

DockTreeModel::DockTreeModel (QObject * parent, tree_data_t * data)
	: TreeModel<DockedInfo>(parent, data)
{
	qDebug("%s", __FUNCTION__);
}

DockTreeModel::~DockTreeModel ()
{
}


QModelIndex DockTreeModel::insertItemWithPath (QStringList const & path, bool checked)
{
	QString const name = path.join("/");
	DockedInfo const * i = 0;
	node_t const * node = m_tree_data->is_present(name, i);
	if (node)
	{
		//qDebug("%s path=%s already present", __FUNCTION__, path.toStdString().c_str());
		return QModelIndex();
	}
	else
	{
		//qDebug("%s path=%s not present, adding", __FUNCTION__, path.toStdString().c_str());
		DockedInfo i;
		i.m_state = checked ? Qt::Checked : Qt::Unchecked;
		i.m_collapsed = false;
		i.m_path = path;
	
		node_t * const n = m_tree_data->set_to_state(name, i);

		QModelIndex const idx = indexFromItem(n);
		setData(idx, checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
		return idx;
	}
}

int DockTreeModel::columnCount (QModelIndex const & parent) const
{
	return e_max_action_type; // @TODO: not supported yet
}

QVariant DockTreeModel::data (QModelIndex const & index, int role) const
{
	if (!index.isValid())
		return QVariant();
	
	int const col = index.column();

	if (col == e_Visibility)
		return TreeModel<DockedInfo>::data(index, role);

	if (col == e_SyncGroup && role == Qt::DisplayRole)
	{
		node_t const * const item = itemFromIndex(index);
		return QString("0");
		//return static_cast<Qt::CheckState>(item->data.m_state);
	}

/*	if (col == e_InCentralWidget && role == Qt::DisplayRole)
	{
		node_t const * const item = itemFromIndex(index);
		return item->data.m_centralwidget;
		//return static_cast<Qt::CheckState>(item->data.m_state);
	}



      e_Visibility = 0
    , e_InCentralWidget
    , e_SyncGroup
    , e_Select
    , e_AlignH
    , e_AlignV
    , e_max_action_type*/

	/*node_t const * const item = itemFromIndex(index);
	if (role == Qt::DisplayRole)
	{
		return QVariant(item->key);
	}
	else if (role == Qt::UserRole) // collapsed or expanded?
	{
		return static_cast<bool>(item->data.m_collapsed);
	}
	else if (role == Qt::CheckStateRole)
	{
		return static_cast<Qt::CheckState>(item->data.m_state);
	}*/
	return QVariant();
}

bool DockTreeModel::setData (QModelIndex const & index, QVariant const & value, int role)
{
	if (!index.isValid()) return false;

	node_t * const item = itemFromIndex(index);
	if (role <= Qt::UserRole)
	{
		return TreeModel<DockedInfo>::setData(index, value, role);
	}
	else if (role == e_DockRoleCentralWidget)
	{
		bool const v = value.toBool();
		item->data.m_centralwidget = v;
	}
	else if (role == e_DockRoleSyncGroup)
	{
	}
	else if (role == e_DockRoleSelect)
	{
	}
	else
		return false;

	emit dataChanged(index, index);
	return true;
}


