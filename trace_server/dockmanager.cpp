#include "dock.h"
#include "mainwindow.h"
#include "connection.h"
#include "dockmanagerconfig.h"
#include "controlbar_dockmanager.h"
#include "serialize.h"
#include <ui_controlbarcommon.h>
/*#include <ui_controlbarlogs.h>
#include <ui_controlbarplots.h>
#include <ui_controlbartables.h>
#include <ui_controlbargantts.h>*/

DockManager::DockManager (MainWindow * mw, QStringList const & path)
	: DockManagerView(mw), ActionAble(path)
	, m_main_window(mw)
	, m_docked_widgets(0)
	, m_control_bar(0)
	, m_model(0)
	, m_config(g_traceServerName)
{
	qDebug("%s", __FUNCTION__);
	resizeColumnToContents(0);

	QString const name = path.join("/");
	QDockWidget * const dock = new QDockWidget(this);
	dock->setObjectName(name);
	dock->setWindowTitle(name);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_main_window->addDockWidget(Qt::BottomDockWidgetArea, dock);
	m_actionables.insert(name, this);
	dock->setAttribute(Qt::WA_DeleteOnClose, false);
	dock->setWidget(this);

	//if (visible)
	//	m_main_window->restoreDockWidget(dock);
	m_docked_widgets = dock;
	m_control_bar = new ControlBarCommon();

	connect(header(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onColumnResized(int, int, int)));
	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
	connect(m_docked_widgets, SIGNAL(dockClosed()), mw, SLOT(onDockManagerClosed()));
	setAllColumnsShowFocus(false);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	//setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
	//header()->setSectionResizeMode(0, QHeaderView::Interactive);
	header()->setStretchLastSection(false);
	setStyleSheet("QTableView::item{ selection-background-color: #F5DEB3  } QTableView::item{ selection-color: #000000 }");
}

void DockManager::loadConfig (QString const & cfgpath)
{
	QString const fname = cfgpath + "/" + g_dockManagerTag;
	DockManagerConfig config2(g_traceServerName);
	if (!::loadConfigTemplate(config2, fname))
	{
		m_config.defaultConfig();
		bool const on = true;
		m_model = new DockManagerModel(this, &m_config.m_data);
		setModel(m_model);
		addActionTreeItem(*this, on);
	}
	else
	{
		m_config = config2;
		config2.m_data.root = 0; // @TODO: promyslet.. takle na to urcite zapomenu
		m_model = new DockManagerModel(this, &m_config.m_data);
		setModel(m_model);
	}
}

void DockManager::applyConfig ()
{
	for (int i = 0, ie = m_config.m_columns_sizes.size(); i < ie; ++i)
	{
		header()->resizeSection(i, m_config.m_columns_sizes[i]);
	}

	if (m_model)
		m_model->syncExpandState(this);
}

void DockManager::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_dockManagerTag;
	::saveConfigTemplate(m_config, fname);
}

DockManager::~DockManager ()
{
	disconnect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
}

DockWidget * DockManager::mkDockWidget (DockedWidgetBase & dwb, bool visible)
{
	return mkDockWidget(dwb, visible, Qt::BottomDockWidgetArea);
}

DockWidget * DockManager::mkDockWidget (ActionAble & aa, bool visible, Qt::DockWidgetArea area)
{
	qDebug("%s", __FUNCTION__);
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
	return dock;
}

void DockManager::onWidgetClosed (DockWidget * w)
{
	qDebug("%s w=%08x", __FUNCTION__, w);
	m_widgets.remove(w->objectName());
}

QModelIndex DockManager::addDockedTreeItem (DockedWidgetBase & dwb, bool on)
{
	m_dockables.insert(dwb.joinedPath(), &dwb);
	return addActionTreeItem(dwb, on);
}

QModelIndex DockManager::addActionTreeItem (ActionAble & aa, bool on)
{
	QModelIndex const idx = m_model->insertItemWithPath(aa.path(), on);
	QModelIndex const idx1 = m_model->index(idx.row(), 1, idx.parent());
	setIndexWidget(idx1, aa.controlWidget());

	// @TODO: set type=AA into returned node
	m_model->setData(idx, QVariant(on ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
	QString const & name = aa.joinedPath();
	m_actionables.insert(name, &aa);
	aa.m_idx = idx;
	//resizeColumnToContents(0);
	return idx;
}

ActionAble const * DockManager::findActionAble (QString const & dst_joined) const
{
	actionables_t::const_iterator it = m_actionables.find(dst_joined);
	if (it != m_actionables.end() && it.key() == dst_joined)
		return *it;
	return 0;
}

/*DockedWidgetBase const * DockManager::findDockableForWidget (QWidget * w) const
{
	for (dockables_t::const_iterator it = m_dockables.begin(), ite = m_dockables.end(); it != ite; ++it)
	{
		DockedWidgetBase const * const dwb = *it;
		DockedWidgetBase const * const dwbw = qobject_cast<DockedWidgetBase const *>(w);
		if (w && dwb && dwb == dwbw)
			return dwb;
	}
	return 0;
}
DockedWidgetBase * DockManager::findDockableForWidget (QWidget * w)
{
	for (dockables_t::const_iterator it = m_dockables.begin(), ite = m_dockables.end(); it != ite; ++it)
	{
		DockedWidgetBase * const dwb = *it;
		if (w && dwb && dwb == qobject_cast<DockedWidgetBase *>(w))
			return dwb;
	}
	return 0;
}*/

void DockManager::removeDockable (QString const & dst_joined)
{
	qDebug("%s", __FUNCTION__);
	dockables_t::iterator it = m_dockables.find(dst_joined);
	if (it != m_dockables.end() && it.key() == dst_joined)
		m_dockables.erase(it);
}

void DockManager::removeActionAble (QString const & dst_joined)
{
	qDebug("%s", __FUNCTION__);
	actionables_t::iterator it = m_actionables.find(dst_joined);
	if (it != m_actionables.end() && it.key() == dst_joined)
		m_actionables.erase(it);
}

DockedWidgetBase const * DockManager::findDockable (QString const & dst_joined) const
{
	dockables_t::const_iterator it = m_dockables.find(dst_joined);
	if (it != m_dockables.end() && it.key() == dst_joined)
		return *it;
	return 0;
}
DockedWidgetBase * DockManager::findDockable (QString const & dst_joined)
{
	dockables_t::iterator it = m_dockables.find(dst_joined);
	if (it != m_dockables.end() && it.key() == dst_joined)
		return *it;
	return 0;
}

void DockManager::onColumnResized (int idx, int , int new_size)
{
	if (idx < 0) return;
	int const curr_sz = m_config.m_columns_sizes.size();
	if (idx < curr_sz)
	{
		//qDebug("%s this=0x%08x hsize[%i]=%i", __FUNCTION__, this, idx, new_size);
	}
	else
	{
		m_config.m_columns_sizes.resize(idx + 1);
		for (int i = curr_sz; i < idx + 1; ++i)
			m_config.m_columns_sizes[i] = 32;
	}
	m_config.m_columns_sizes[idx] = new_size;
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
				qDebug("delivering action to: %s", dst_joined.toStdString().c_str());
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

void DockManager::onClicked (QModelIndex idx)
{
	TreeModel<DockedInfo>::node_t const * n = m_model->getItemFromIndex(idx);
	QStringList const & dst = n->data.m_path;

	int const col = idx.column();
	Action a;
	a.m_type = static_cast<E_ActionType>(col);
	a.m_src_path = path();
	a.m_src = this;
	a.m_dst_path = dst;
	if (col == e_Visibility)
	{
		int const state = m_model->data(idx, Qt::CheckStateRole).toInt();
		a.m_args.push_back(state);
	}
	if (col == e_InCentralWidget)
	{
		int const state = m_model->data(idx, e_DockRoleCentralWidget).toInt();
		int const new_state = state == 0 ? 1 : 0;

		m_model->setData(idx, new_state, e_DockRoleCentralWidget);
		a.m_args.push_back(new_state);
	}

	//av.m_dst = 0;
	handleAction(&a, e_Sync);
}

////////////////////////////////////////
/*QString DockManager::matchClosestPresetName (QString const & app_name)
{
	QString const top_level_preset = m_main_window->getCurrentPresetName();

	return top_level_preset;

	QString const multitab_preset_hint = ui->multiTabPresetComboBox->currentText();
	if (!multitab_preset_hint.isEmpty())
	{
		return app_name + "/" + multitab_preset_hint;
	}


	QString const saved_preset = getCurrentPresetName();
	QString preset_appname = saved_preset;
	int const slash_pos = preset_appname.lastIndexOf(QChar('/'));
	if (slash_pos != -1)
		preset_appname.chop(preset_appname.size() - slash_pos);
	qDebug("match preset name: curr=%s app_name=%s", preset_appname.toStdString().c_str(), app_name.toStdString().c_str());
	if (preset_appname.contains(app_name))
	{
		qDebug("got correct preset name appname/.* from combobox");
		return saved_preset;
	}
	else
	{
		qDebug("got nonmatching preset name appname/.* from combobox");
		return QString();
	}
}

*/
