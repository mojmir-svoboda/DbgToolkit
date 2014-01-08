#pragma once
#include <QWidget>
#include <QTreeView>
#include <QComboBox>
#include <QListView>
#include "filterstate.h"
#include "treemodel.h"
#include "treeview.h"
#include "delegates.h"
#include <boost/tuple/tuple.hpp>

namespace Ui {	class FilterWidget; }
class TreeProxyModel;

class FilterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FilterWidget (QWidget * parent = 0);
    ~FilterWidget ();
    
	friend class PersistentFilter;
    Ui::FilterWidget * ui;

	void loadConfig (QString const & path, FilterState & config);
	void loadConfig (QString const & path);
	void saveConfig (QString const & path);
	void applyConfig (FilterState const & src, FilterState & dst);

	// filtering
	void onRegexActivate (int idx);
	void onRegexAdd ();
	void onRegexRm ();
	//void onStringActivate (int idx);
	void onStringAdd ();
	void onStringRm ();
	void onGotoRegexFilter ();
	void onGotoLevelFilter ();
	void syncSettingsViews (QListView const * const invoker, QModelIndex const idx);
	void onClickedAtCtxTree (QModelIndex idx);
	void onDoubleClickedAtCtxTree (QModelIndex idx);
	void onClickedAtTIDList (QModelIndex idx);
	void onDoubleClickedAtTIDList (QModelIndex idx);
	void onClickedAtLvlList (QModelIndex idx);
	void onDoubleClickedAtLvlList (QModelIndex idx);
	void onClickedAtRegexList (QModelIndex idx);
	void onDoubleClickedAtRegexList (QModelIndex idx);
	void onClickedAtStringList (QModelIndex idx);
	void onDoubleClickedAtStringList (QModelIndex idx);


	// sem??
	void onHidePrevFromRow ();
	void onUnhidePrevFromRow ();
	void onToggleRefFromRow ();
	void onClearCurrentView ();
	//void onClearCurrentFileFilter ();
	void onClearCurrentCtxFilter ();
	void onClearCurrentTIDFilter ();
	void onClearCurrentRegexFilter ();
	void onClearCurrentStringFilter ();
	void onClearCurrentScopeFilter ();
	void onClearCurrentRefTime ();
	void onSelectAllLevels ();
	void onSelectNoLevels ();
	void onSelectAllCtxs ();
	void onSelectNoCtxs ();

	FilterState		m_filter_state;

	//QItemSelectionModel * m_proxy_selection;
	QStandardItemModel * m_func_model;
};

bool loadFilterState (FilterState & s, std::string const & filename);
bool loadFilterState (FilterState const & src, FilterState & target);
