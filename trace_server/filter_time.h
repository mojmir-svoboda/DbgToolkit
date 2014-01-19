#pragma once
#include "filterbase.h"
#include "ui_filter_time.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

enum E_CmpMode {
	  e_CmpL = 0
	, e_CmpLE
	, e_CmpE
	, e_CmpGE
	, e_CmpG
	, e_CmpNE
	, e_max_cmpmod_enum_value
};

struct FilteredTime {
	QString m_string;
	QString m_time_units_str;
    unsigned long long m_src_value; // client's natural units are microseconds
    float m_value;  // but float is more natural to user
    float m_time_units;
	bool m_is_enabled;
	int m_operator;

	bool match (float f) const;

	FilteredTime () { }
	FilteredTime (QString const & op, QString const & rhs, QString const & units, bool enabled);
	FilteredTime (QString const & op, QString const & rhs, QString const & units);

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("string", m_string);
		ar & boost::serialization::make_nvp("time_units_str", m_time_units_str);
		ar & boost::serialization::make_nvp("srcvalue", m_src_value);
		ar & boost::serialization::make_nvp("value", m_value);
		ar & boost::serialization::make_nvp("time_units", m_time_units);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("operator", m_operator);
	}
};

inline bool operator== (FilteredTime const & lhs, FilteredTime const & rhs)
{
    return lhs.m_string == rhs.m_string && lhs.m_operator == rhs.m_operator && lhs.m_time_units_str == rhs.m_time_units_str;
}


struct FilterTime : FilterBase
{
	Ui_FilterTime * m_ui;

	FilterTime (QWidget * parent = 0);
	virtual ~FilterTime ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Time; }

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
		ar & boost::serialization::make_nvp("filtered_strings", m_data);
	}

	// time filtering
	void setupModel ();
	void destroyModel ();
    void append (QString const & op, QString const & s, QString const & units, bool enabled);
	void appendToWidgets (FilteredTime const & flt);
    void remove (QString const & op, QString const & s, QString const & u);
	bool isMatchedTimeExcluded (QString str) const;
	void setChecked (QString const & s, bool checked);
	void setOperator (QString const & s, int state);
	void recompile ();
	void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }

	QList<FilteredTime>	    m_data;
	QStandardItemModel *	m_model;
	QStyledItemDelegate *   m_delegate;

	Q_OBJECT
public slots:
	void onClickedAt (QModelIndex idx);
	void onRm ();
	void onAdd ();
    void onAdd (QString const & op, QString const & rhs, QString const & units);
signals:

};

struct TimeDelegate : public QStyledItemDelegate
{
    TimeDelegate (QObject * parent = 0) : QStyledItemDelegate(parent) { }
    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

