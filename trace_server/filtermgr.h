#pragma once
#include "filterbase.h"
#include <QTabWidget>

struct FilterMgr : FilterBase 
{
	typedef std::vector<FilterBase *> filters_t;
	filters_t m_filters;

	FilterMgr (QWidget * parent = 0);
	virtual ~FilterMgr ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Mgr; }

	virtual bool accept (DecodedCommand const & cmd) const;

	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();

	void addFilter (FilterBase * b);
	void rmFilter (FilterBase * b);
	void mvFilter (int from, int to);

    QTabWidget * m_tabFilters;
	Q_OBJECT
};

