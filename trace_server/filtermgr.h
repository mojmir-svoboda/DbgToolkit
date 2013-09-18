#pragma once
#include "filterbase.h"
#include <QTabWidget>
#include "combolist.h"
#include "movablelistmodel.h"
#include "movabletabwidget.h"
//#include "filter_script.h"
#include "filter_string.h"
#include "filter_regex.h"
#include "filter_ctx.h"
#include "filter_lvl.h"
#include "filter_tid.h"
#include "filter_fileline.h"

struct FilterMgr : FilterBase 
{
	QStringList 				m_filter_order;
	typedef std::vector<FilterBase *> filters_t;
	filters_t					m_filters;	/// user-order respecting filters
	std::vector<FilterBase *> 	m_cache; // enum ordered cache of m_filters

	FilterMgr (QWidget * parent = 0);
	virtual ~FilterMgr ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Mgr; }

	virtual bool accept (DecodedCommand const & cmd) const;

	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();
	virtual void clear ();

	void addFilter (FilterBase * b);
	void rmFilter (FilterBase * b);
	void mvFilter (int from, int to);
	void recreateFilters ();

	//FilterXX *			getFilterXX () { return static_cast<FilterXX *>(m_cache[e_Filter_XX]); }
	//FilterXX const *		getFilterXX () const { return static_cast<FilterXX const *>(m_cache[e_Filter_XX]); }
	//FilterScript *		getFilterScript () { return static_cast<FilterScript *>(m_cache[e_Filter_Script]); }
	//FilterScript const *	getFilterScript () const { return static_cast<FilterScript const *>(m_cache[e_Filter_Script]); }
	FilterString *			getFilterString () { return static_cast<FilterString *>(m_cache[e_Filter_String]); }
	FilterString const *	getFilterString () const { return static_cast<FilterString const *>(m_cache[e_Filter_String]); }
	FilterRegex *			getFilterRegex () { return static_cast<FilterRegex *>(m_cache[e_Filter_Regex]); }
	FilterRegex const *		getFilterRegex () const { return static_cast<FilterRegex const *>(m_cache[e_Filter_Regex]); }
	FilterCtx *				getFilterCtx () { return static_cast<FilterCtx *>(m_cache[e_Filter_Ctx]); }
	FilterCtx const *		getFilterCtx () const { return static_cast<FilterCtx const *>(m_cache[e_Filter_Ctx]); }
	FilterLvl *				getFilterLvl () { return static_cast<FilterLvl *>(m_cache[e_Filter_Lvl]); }
	FilterLvl const *		getFilterLvl () const { return static_cast<FilterLvl const *>(m_cache[e_Filter_Lvl]); }
	FilterTid *				getFilterTid () { return static_cast<FilterTid *>(m_cache[e_Filter_Tid]); }
	FilterTid const *		getFilterTid () const { return static_cast<FilterTid const *>(m_cache[e_Filter_Tid]); }
	FilterFileLine *		getFilterFileLine () { return static_cast<FilterFileLine *>(m_cache[e_Filter_FileLine]); }
	FilterFileLine const *	getFilterFileLine () const { return static_cast<FilterFileLine const *>(m_cache[e_Filter_FileLine]); }

	void clearUI ();
	void setConfigToUI ();
	void setUIToConfig ();

public slots:
	void onShowContextMenu (QPoint const & pt);
	void onHideContextMenu ();
	void onCtxAddButton ();
	void onCtxRmButton ();
	void onTabMoved (int from, int to);

public:
    MovableTabWidget *		m_tabFilters;
	ComboList *				m_tabCtxMenu;
	QStyledItemDelegate *	m_delegate;
	MyListModel *			m_tabCtxModel;
	Q_OBJECT
};

bool loadConfig (FilterMgr & config, QString const & fname);
bool saveConfig (FilterMgr const & config, QString const & fname);
void fillDefaultConfig (FilterMgr & config);
