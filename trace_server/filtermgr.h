#pragma once
#include "filterbase.h"
#include <QTabWidget>
#include "combolist.h"
#include "movablelistmodel.h"
#include "movabletabwidget.h"
#include "filter_script.h"
#include "filter_string.h"
#include "filter_regex.h"
#include "filter_ctx.h"
#include "filter_lvl.h"
#include "filter_tid.h"
#include "filter_fileline.h"
#include "filter_row.h"

struct FilterMgrBase : FilterBase 
{
	QStringList 				m_filter_order;
	typedef QList<FilterBase *> filters_t;
	filters_t					m_filters;	/// user-order respecting filters

	FilterMgrBase (QWidget * parent = 0);
	virtual ~FilterMgrBase () = 0;

	virtual void initUI () = 0;
	virtual void doneUI () = 0;

	virtual E_FilterType type () const = 0;

	virtual bool accept (DecodedCommand const & cmd) const;
	virtual bool enabled () const;
	virtual bool someFilterEnabled () const;
	virtual void addFilter (FilterBase * b);
	virtual void rmFilter (FilterBase * & b);
	virtual void mvFilter (int from, int to);
	virtual void recreateFilters () = 0;

	virtual void defaultConfig () = 0;
	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();
	virtual void clear () = 0;

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterBase::serialize(ar, version);
		ar & boost::serialization::make_nvp("filter_order", m_filter_order);
		ar & boost::serialization::make_nvp("curr_tab", m_currTab);
	}

	void connectFiltersTo (QWidget * w);
	void disconnectFiltersTo (QWidget * w);

	void clearUI ();
	void setConfigToUI ();
	void setUIToConfig ();
	void focusToFilter (E_FilterType type);

public slots:
	void onFilterEnabledChanged ();
	void onShowContextMenu (QPoint const & pt);
	void onHideContextMenu ();
	void onCtxAddButton ();
	void onCtxRmButton ();
	void onCtxCommitButton ();
	void onTabMoved (int from, int to);
signals:

public:
    MovableTabWidget *		m_tabFilters;
	ComboList *				m_tabCtxMenu;
	QStyledItemDelegate *	m_delegate;
	MyListModel *			m_tabCtxModel;
	int						m_currTab;
	Q_OBJECT
};

struct FilterMgr : FilterMgrBase
{
	std::vector<FilterBase *> 	m_cache;	/// enum ordered cache of m_filters

	FilterMgr (QWidget * parent = 0);
	virtual ~FilterMgr ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Mgr; }

	virtual void defaultConfig ();
	virtual void clear ();

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterMgrBase::serialize(ar, version);
	}

	virtual void addFilter (FilterBase * b);
	virtual void rmFilter (FilterBase * & b);
	virtual void recreateFilters ();

	//FilterXX *			getFilterXX () { return static_cast<FilterXX *>(m_cache[e_Filter_XX]); }
	//FilterXX const *		getFilterXX () const { return static_cast<FilterXX const *>(m_cache[e_Filter_XX]); }
	FilterRow *			getFilterRow () { return static_cast<FilterRow *>(m_cache[e_Filter_Row]); }
	FilterRow const *		getFilterRow () const { return static_cast<FilterRow const *>(m_cache[e_Filter_Row]); }
	FilterScript *		getFilterScript () { return static_cast<FilterScript *>(m_cache[e_Filter_Script]); }
	FilterScript const *	getFilterScript () const { return static_cast<FilterScript const *>(m_cache[e_Filter_Script]); }
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

	//void clearUI ();
	//void setConfigToUI ();
	//void setUIToConfig ();

public slots:
	void onShowContextMenu (QPoint const & pt);
	void onHideContextMenu ();
	void onCtxAddButton ();
	void onCtxRmButton ();
	void onCtxCommitButton ();
	void onTabMoved (int from, int to);
signals:

public:
    MovableTabWidget *		m_tabFilters;
	ComboList *				m_tabCtxMenu;
	QStyledItemDelegate *	m_delegate;
	MyListModel *			m_tabCtxModel;
	int						m_currTab;
	Q_OBJECT
};

