#pragma once
#include "filterbase.h"
#include "ui_filter_regex.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

struct FilteredRegex {
	QString m_regex_str;
	QRegExp m_regex;
	bool m_is_enabled;
	int m_state;

	bool isValid () const { return m_regex.isValid(); }
	bool exactMatch (QString str) const { return m_regex.exactMatch(str); }

	FilteredRegex () { }
	FilteredRegex (QString const & rs, bool enabled, int state)
        : m_regex_str(rs), m_regex(rs), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("regex_str", m_regex_str);
		ar & boost::serialization::make_nvp("regex", m_regex);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

struct FilterRegex : FilterBase
{
	Ui_FilterRegex * m_ui;

	FilterRegex (QWidget * parent = 0);
	virtual ~FilterRegex ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Regex; }

	virtual bool accept (DecodedCommand const & cmd) const;

	virtual void defaultConfig ();
	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();
	virtual void clear ();

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterBase::serialize(ar, version);
		ar & boost::serialization::make_nvp("filtered_regexps", m_data);
	}

	// regex filtering
	void setupModel ();
	void destroyModel ();
	void appendToRegexFilters (QString const & str, bool checked, bool inclusive);
	void removeFromRegexFilters (QString const & str);
	bool isMatchedRegexExcluded (QString str) const;
	void setRegexChecked (QString const & s, bool checked);
	void setRegexInclusive (QString const & s, bool inclusive);
	void recompile ();
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }

	void onClearRegexFilter () { m_data.clear(); }
	QList<FilteredRegex>	m_data;
	QStandardItemModel *	m_model;
	QStyledItemDelegate *   m_delegate;

	Q_OBJECT
public slots:
	void onClickedAtRegexList (QModelIndex idx);
	void onRegexActivate (int);
	void onRegexAdd ();
	void onStringAdd ();
	void onRegexRm ();
signals:
};

struct RegexDelegate : public QStyledItemDelegate
{
    RegexDelegate (QObject * parent = 0) : QStyledItemDelegate(parent) { }
    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

