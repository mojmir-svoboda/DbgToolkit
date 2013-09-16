#pragma once
#include "filterbase.h"
#include "ui_filter_lvl.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>

struct FilterLvl : FilterBase
{
	Ui_FilterLvl * m_ui;

	FilterLvl (QWidget * parent = 0);
	virtual ~FilterLvl ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Lvl; }

	virtual bool accept (DecodedCommand const & cmd) const;

	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();

	Q_OBJECT

	// lvl
	typedef QList<FilteredLevel> lvl_filters_t;
	void appendLvlFilter (QString const & item);
	void removeLvlFilter (QString const & item);
	bool isLvlPresent (QString const & item, bool & enabled, E_LevelMode & lvlmode) const;
	bool setLvlMode (QString const & item, bool enabled, E_LevelMode lvlmode);
	void onClearLvlFilter () { m_lvl_filters.clear(); }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("lvl_filters", m_lvl_filters);
	}
	lvl_filters_t			m_lvl_filters;

};
