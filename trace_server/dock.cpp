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
	: QObject(mw), ActionAble(path)
	, m_main_window(mw)
	, m_docked_widgets(0)
	, m_docked_widgets_tree_view(0)
	, m_docked_widgets_model(0)
{

	m_docked_widgets_tree_view->setModel(m_docked_widgets_model);
	connect(m_docked_widgets_tree_view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtDockedWidgets(QModelIndex)));
	connect(m_docked_widgets, SIGNAL(visibilityChanged(bool)), this, SLOT(onListVisibilityChanged(bool)));
	connect(m_docked_widgets, SIGNAL(dockClosed()), m_main_window, SLOT(onDockManagerClosed()));
}

DockManager::~DockManager ()
{
	disconnect(m_docked_widgets_tree_view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtDockedWidgets(QModelIndex)));
}

DockWidget * DockManager::mkDockWidget (DockedWidgetBase & dwb, bool visible)
{
	return mkDockWidget(dwb, visible, Qt::BottomDockWidgetArea);
}

DockWidget * DockManager::mkDockWidget (DockedWidgetBase & dwb, bool visible, Qt::DockWidgetArea area)
{
	QString const name = dwb.dockPath().join("/");
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
	QModelIndex const idx = m_docked_widgets_model->insertItemWithPath(dwb.dockPath(), on);
	m_docked_widgets_model->setData(idx, QVariant(on ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
	dwb.m_idx = idx;
	return idx;
}


bool DockManager::handleAction (Action * a, bool sync)
{
	QStringList const & src_path = a->m_src_path;
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
	handleAction(&av, true);
	
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
	
		node_t * const n = m_tree_data->set_to_state(name, i);

		QModelIndex const idx = indexFromItem(n);
		setData(idx, checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
		return idx;
	}
}

