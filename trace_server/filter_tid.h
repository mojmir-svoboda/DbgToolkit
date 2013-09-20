#pragma once
#include "filterbase.h"
#include "ui_filter_tid.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

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
		ar & boost::serialization::make_nvp("enabled", m_enabled);
	}

	// tid specific
	void setupModel ();
	void destroyModel ();
	void appendTIDFilter (QString const & item);
	void removeTIDFilter (QString const & item);
	bool isTIDExcluded (QString const & item) const;
	void recompile ();

	typedef std::vector<QString> tid_filters_t;
	tid_filters_t			m_data;
	QStandardItemModel *	m_model;

	Q_OBJECT
public slots:
	void onClickedAtTIDList (QModelIndex idx);
signals:
	void filterChangedSignal ();
};
