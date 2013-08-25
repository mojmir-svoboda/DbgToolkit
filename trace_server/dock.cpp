#include "dock.h"
#include <QString>
#include <QWidget>
#include <QDockWidget>
#include <QMultiMap>
#include <QCloseEvent>
#include <QMainWindow>
#include "mainwindow.h"
#include "connection.h"

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

// @TODO: hmm. this whole fn is.. unfortunately rushed. need to rethink
void DockManager::onClickedAtDockedWidgets (QModelIndex idx)
{
	TreeModel<DockedInfo>::node_t const * n = m_docked_widgets_model->getItemFromIndex(idx);
	QStringList const & dst = n->data.m_path;
	int const state = m_docked_widgets_model->data(idx, Qt::CheckStateRole).toInt();

	ActionVisibility av;
	av.m_args.push_back(state);
	av.m_src_path = path();
	av.m_src = this;
	av.m_dst_path = dst;
	//av.m_dst = 0;
	handleAction(&av, e_Sync);
	
/*	QList<QString> path;
	QList<bool> state;
	path.push_front(m_docked_widgets_model->data(idx).toString());
	state.push_front(model->data(idx, Qt::CheckStateRole).toInt());
	QModelIndex parent = model->parent(idx);
	while (parent.isValid())
	{
		path.push_front(model->data(parent).toString());
		state.push_front(model->data(parent, Qt::CheckStateRole).toInt());
		parent = model->parent(parent);
	}

	//path[0]=WarHorse_App path[1]=table path[2]=pokus path[3]=--
	qDebug("path[0]=%s", path.size() > 0 ? path.at(0).toStdString().c_str() : "--");
	qDebug("path[1]=%s", path.size() > 1 ? path.at(1).toStdString().c_str() : "--");
	qDebug("path[2]=%s", path.size() > 2 ? path.at(2).toStdString().c_str() : "--");
	qDebug("path[3]=%s", path.size() > 3 ? path.at(3).toStdString().c_str() : "--");

	Q_ASSERT(path.size());

	if (Connection * conn = findConnectionByName(path.at(0)))
	{
		if (path.size() > 1)
		{
			QString class_type = path.at(1);
			if (class_type == "table")
			{
				//path.pop_front(); // drop app name
				//path.pop_front(); // drop widget identifier

				if (path.size() > 2)
				{
					for (datatables_t::iterator it = conn->m_data.get<e_data_table>().begin(), ite = conn->m_data.get<e_data_table>().end(); it != ite; ++it)
					{
						DataTable * dp = (*it);
						if (dp->m_config.m_tag == path.at(2))
						{
							bool apply = false;
							bool const xchg = dp->widget().getConfig().m_show ^ state.at(2);
							apply |= xchg;
							if (xchg)
							{
								dp->m_config.m_show = state.at(2);
							}

							if (state.at(2))
								dp->onShow();
							else
								dp->onHide();

							if (apply)
								dp->widget().applyConfig(dp->widget().getConfig());
						}
					}
				}
				else
				{
					if (state.at(1))
						conn->onShowTables();
					else
						conn->onHideTables();
				}
			}
			else
			{
				if (path.size() > 2)
				{
					for (dataplots_t::iterator it = conn->m_data.get<e_data_plot>().begin(), ite = conn->m_data.get<e_data_plot>().end(); it != ite; ++it)
					{
						DataPlot * dp = (*it);
						if (dp->m_config.m_tag == path.at(2))
						{
							bool apply = false;
							bool const xchg = dp->widget().getConfig().m_show ^ state.at(2);
							apply |= xchg;
							if (xchg)
							{
								dp->m_config.m_show = state.at(2);
							}

							if (path.size() > 3)
							{
								for (int cc = 0, cce = dp->m_config.m_ccfg.size(); cc < cce; ++cc)
								{
									plot::CurveConfig & cfg = dp->m_config.m_ccfg[cc];
									if (cfg.m_tag == path.at(3))
									{
										apply |= cfg.m_show ^ state.at(3);
										cfg.m_show = state.at(3);
										break;
									}
								}
							}
							else if (path.size() > 2)
							{
								for (int cc = 0, cce = dp->m_config.m_ccfg.size(); cc < cce; ++cc)
								{
									plot::CurveConfig & cfg = dp->m_config.m_ccfg[cc];
									apply |= cfg.m_show ^ state.at(2);
									cfg.m_show = state.at(2);
								}

								if (state.at(2))
									dp->onShow();
								else
									dp->onHide();
							}

							if (apply)
							{
								dp->widget().applyConfig(dp->widget().getConfig());
							}
						}
					}
				}
				else
				{
					if (state.at(1))
						conn->onShowPlots();
					else
						conn->onHidePlots();
				}
			}
		}

	}
*/

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

