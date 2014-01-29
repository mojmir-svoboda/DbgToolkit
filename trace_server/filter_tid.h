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
		FilterBase::serialize(ar, version);
	}

	// tid specific
	void setupModel ();
	void destroyModel ();
	void append (QString const & item);
	void remove (QString const & item);
	bool isTIDExcluded (QString const & item) const;
	void recompile ();
	QListView * getWidget () { return m_ui->view; }
	QListView const * getWidget () const { return m_ui->view; }
	void locateItem (QString const & item, bool scrollto, bool expand);

	typedef std::vector<QString> tid_filters_t;
	tid_filters_t			m_data;
	QStandardItemModel *	m_model;

	Q_OBJECT
public slots:
	void onClicked (QModelIndex idx);
	void onSelectAll ();
	void onSelectNone ();
signals:
	void filterChangedSignal ();
};
