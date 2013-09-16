#pragma once
#include "filterbase.h"
#include "ui_filter_tid.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>

struct FilterTid : FilterBase
{
	Ui_FilterTid * m_ui;

	FilterTid (QWidget * parent = 0);
	virtual ~FilterTid ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Tid; }

	virtual bool accept (DecodedCommand const & cmd) const;

	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();

	Q_OBJECT

	// tid
	typedef std::vector<QString> tid_filters_t;
	void appendTIDFilter (QString const & item);
	void removeTIDFilter (QString const & item);
	bool isTIDExcluded (QString const & item) const;

	void onClearTIDFilter () { m_tid_filters.clear(); }
	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
	}

	tid_filters_t			m_tid_filters;

};
