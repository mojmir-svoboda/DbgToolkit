#pragma once
#include "filterbase.h"
#include "ui_filter_string.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>

struct FilterString : FilterBase
{
	Ui_FilterString * m_ui;

	FilterString (QWidget * parent = 0);
	virtual ~FilterString ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_String; }

	virtual bool accept (DecodedCommand const & cmd) const;

	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();

	Q_OBJECT

	// string filtering
	void appendToStringFilters (QString const & str, bool checked, int state);
	void removeFromStringFilters (QString const & str);
	bool isMatchedStringExcluded (QString str) const;
	void setStringChecked (QString const & s, bool checked);
	void setStringState (QString const & s, int state);

	void onClearStringFilter () { m_filtered_strings.clear(); }
	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("filtered_strings", m_filtered_strings);
	}
	QList<FilteredString>	m_filtered_strings;

};
