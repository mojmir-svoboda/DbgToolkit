#pragma once
#include "filterbase.h"
#include "ui_filter_tid.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

struct FilteredTid {
	QString m_tid_str;
	unsigned long long m_tid;
	bool m_is_enabled;
	int m_state;

	FilteredTid () { }
	FilteredTid (QString tid, bool enabled, int state)
        : m_tid_str(tid), m_tid(tid.toInt()), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("tid_str", m_tid_str);
		ar & boost::serialization::make_nvp("tid", m_tid);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

inline bool operator< (FilteredTid const & lhs, FilteredTid const & rhs)
{
	return lhs.m_tid < rhs.m_tid;
}



struct FilterTid : FilterBase
{
	Ui_FilterTid * m_ui;

	FilterTid (QWidget * parent = 0);
	virtual ~FilterTid ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Tid; }

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
	}

	// tid specific
	void setupModel ();
	void destroyModel ();
	void append (QString const & item);
	void remove (QString const & item);
	void recompile ();
	bool isPresent (QString const & item, bool & enabled) const;
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }
	void locateItem (QString const & item, bool scrollto, bool expand);

	typedef std::vector<FilteredTid> tid_filters_t;
	tid_filters_t			m_data;
	QStandardItemModel *	m_model;

	Q_OBJECT
public slots:
	void onClicked (QModelIndex idx);
	void onSelectAll ();
	void onSelectNone ();
signals:
};
