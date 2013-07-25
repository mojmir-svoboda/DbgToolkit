#pragma once
#include <QWidget>
#include <QTreeView>
#include <QComboBox>
#include <QListView>
#include "filterstate.h"
#include "treemodel.h"
#include "treeview.h"
#include "treeproxy.h"
#include "delegates.h"
#include <boost/tuple/tuple.hpp>

namespace Ui {
	class FilterWidget;
}

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

	TreeModel * fileModel () { return m_file_model; }
	TreeModel const * fileModel () const { return m_file_model; }

	TreeView * getWidgetFile ();
	TreeView const * getWidgetFile () const;
	QTreeView * getWidgetCtx ();
	QTreeView const * getWidgetCtx () const;
    QComboBox * getFilterRegex ();
    QComboBox const * getFilterRegex () const;
	QTreeView * getWidgetRegex ();
	QTreeView const * getWidgetRegex () const;
	QTreeView * getWidgetString ();
	QTreeView const * getWidgetString () const;
    QComboBox * getFilterColorRegex ();
    QComboBox const * getFilterColorRegex () const;
	QListView * getWidgetColorRegex ();
	QListView const * getWidgetColorRegex () const;
	QListView * getWidgetTID ();
	QListView const * getWidgetTID () const;
	QTreeView * getWidgetLvl ();
	QTreeView const * getWidgetLvl () const;

	void setupModelFile ();
	void destroyModelFile ();

	// filtering
	void onRegexActivate (int idx);
	void onRegexAdd ();
	void onRegexRm ();
	void onColorRegexActivate (int idx);
	void onColorRegexAdd ();
	void onColorRegexRm ();
	//void onStringActivate (int idx);
	void onStringAdd ();
	void onStringRm ();
	void onGotoFileFilter ();
	void onGotoColorFilter ();
	void onGotoRegexFilter ();
	void onGotoLevelFilter ();
	void syncSettingsViews (QListView const * const invoker, QModelIndex const idx);
	void onFilterFileComboChanged (QString str);
	void onCancelFilterFileButton ();
	void onClickedAtCtxTree (QModelIndex idx);
	void onDoubleClickedAtCtxTree (QModelIndex idx);
	void onClickedAtTIDList (QModelIndex idx);
	void onDoubleClickedAtTIDList (QModelIndex idx);
	void onClickedAtLvlList (QModelIndex idx);
	void onDoubleClickedAtLvlList (QModelIndex idx);
	void onClickedAtRegexList (QModelIndex idx);
	void onDoubleClickedAtRegexList (QModelIndex idx);
	void onClickedAtColorRegexList (QModelIndex idx);
	void onDoubleClickedAtColorRegexList (QModelIndex idx);
	void onClickedAtStringList (QModelIndex idx);
	void onDoubleClickedAtStringList (QModelIndex idx);




	// sem??
	void onHidePrevFromRow ();
	void onUnhidePrevFromRow ();
	void onExcludeFileLine ();
	void onToggleRefFromRow ();
	void onClearCurrentView ();
	void onClearCurrentFileFilter ();
	void onClearCurrentCtxFilter ();
	void onClearCurrentTIDFilter ();
	void onClearCurrentColorizedRegexFilter ();
	void onClearCurrentRegexFilter ();
	void onClearCurrentStringFilter ();
	void onClearCurrentScopeFilter ();
	void onClearCurrentRefTime ();
	void onSelectAllLevels ();
	void onSelectNoLevels ();
	void onSelectAllCtxs ();
	void onSelectNoCtxs ();

	FilterState		m_filter_state;

	TreeModel * m_file_model;
	TreeProxyModel * m_file_proxy;
	QItemSelectionModel * m_proxy_selection;
	QStandardItemModel * m_ctx_model;
	QStandardItemModel * m_func_model;
	QStandardItemModel * m_tid_model;
	QStandardItemModel * m_color_regex_model;
	QStandardItemModel * m_regex_model;
	QStandardItemModel * m_lvl_model;
	QStandardItemModel * m_string_model;

	enum E_Delegates {
		  e_delegate_Level
		, e_delegate_Ctx
		, e_delegate_String
		, e_delegate_Regex
		, e_delegate_max_enum_value
	};
	boost::tuple<LevelDelegate *, CtxDelegate *, StringDelegate *, RegexDelegate *> m_delegates;

};

bool loadFilterState (FilterState & s, std::string const & filename);
bool loadFilterState (FilterState const & src, FilterState & target);
