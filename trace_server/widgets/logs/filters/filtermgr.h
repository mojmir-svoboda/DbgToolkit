#pragma once
#include <filters/filtermgrbase.h>
//#include "filter_script.h"
#include "filter_string.h"
//#include "filter_regex.h"
#include "filter_ctx.h"
#include "filter_lvl.h"
#include "filter_tid.h"
#include "filter_fileline.h"
#include "filter_row.h"
#include "filter_time.h"

namespace logs {

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
	virtual FilterBase * filterFactory (E_FilterType t, QWidget * parent);
	virtual void fillComboBoxWithFilters (QComboBox * cbx);

	//FilterXX *			getFilterXX () { return static_cast<FilterXX *>(m_cache[e_Filter_XX]); }
	//FilterXX const *		getFilterXX () const { return static_cast<FilterXX const *>(m_cache[e_Filter_XX]); }
	FilterRow *				getFilterRow () { return static_cast<FilterRow *>(m_cache[e_Filter_Row]); }
	FilterRow const *		getFilterRow () const { return static_cast<FilterRow const *>(m_cache[e_Filter_Row]); }
// 	FilterScript *			getFilterScript () { return static_cast<FilterScript *>(m_cache[e_Filter_Script]); }
// 	FilterScript const *	getFilterScript () const { return static_cast<FilterScript const *>(m_cache[e_Filter_Script]); }
	FilterString *			getFilterString () { return static_cast<FilterString *>(m_cache[e_Filter_String]); }
	FilterString const *	getFilterString () const { return static_cast<FilterString const *>(m_cache[e_Filter_String]); }
	FilterCtx *				getFilterCtx () { return static_cast<FilterCtx *>(m_cache[e_Filter_Ctx]); }
	FilterCtx const *		getFilterCtx () const { return static_cast<FilterCtx const *>(m_cache[e_Filter_Ctx]); }
	FilterLvl *				getFilterLvl () { return static_cast<FilterLvl *>(m_cache[e_Filter_Lvl]); }
	FilterLvl const *		getFilterLvl () const { return static_cast<FilterLvl const *>(m_cache[e_Filter_Lvl]); }
	FilterTid *				getFilterTid () { return static_cast<FilterTid *>(m_cache[e_Filter_Tid]); }
	FilterTid const *		getFilterTid () const { return static_cast<FilterTid const *>(m_cache[e_Filter_Tid]); }
	FilterFileLine *		getFilterFileLine () { return static_cast<FilterFileLine *>(m_cache[e_Filter_FileLine]); }
	FilterFileLine const *	getFilterFileLine () const { return static_cast<FilterFileLine const *>(m_cache[e_Filter_FileLine]); }
	FilterTime *			getFilterTime () { return static_cast<FilterTime *>(m_cache[e_Filter_Time]); }
	FilterTime const *		getFilterTime () const { return static_cast<FilterTime const *>(m_cache[e_Filter_Time]); }

	FilterBase *			mkFilter (E_FilterType t);

	//void clearUI ();
	//void setConfigToUI ();
	//void setUIToConfig ();

public slots:
signals:

public:
	Q_OBJECT
};

}