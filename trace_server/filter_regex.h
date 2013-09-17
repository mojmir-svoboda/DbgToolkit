#pragma once
#include "filterbase.h"
#include "ui_filter_regex.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

struct FilterRegex : FilterBase
{
	Ui_FilterRegex * m_ui;

	FilterRegex (QWidget * parent = 0);
	virtual ~FilterRegex ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Regex; }

	virtual bool accept (DecodedCommand const & cmd) const;

	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();
	virtual void clear ();

	// regex filtering
	void setupModel ();
	void destroyModel ();
	void appendToRegexFilters (QString const & str, bool checked, bool inclusive);
	void removeFromRegexFilters (QString const & str);
	bool isMatchedRegexExcluded (QString str) const;
	void setRegexChecked (QString const & s, bool checked);
	void setRegexInclusive (QString const & s, bool inclusive);

	void onClearRegexFilter () { m_filtered_regexps.clear(); }
	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("filtered_regexps", m_filtered_regexps);
	}

	QList<FilteredRegex>	m_filtered_regexps;
	QStandardItemModel *	m_regex_model;
	QStyledItemDelegate *   m_delegate;

	Q_OBJECT
};

struct RegexDelegate : public QStyledItemDelegate
{
    RegexDelegate (QObject * parent = 0) : QStyledItemDelegate(parent) { }
    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

