#pragma once
#include "filterbase.h"
#include "ui_filter_lvl.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

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

	virtual void clear ();

	// lvl specific
	void setupModel ();
	void destroyModel ();
	void appendLvlFilter (QString const & item);
	void removeLvlFilter (QString const & item);
	bool isLvlPresent (QString const & item, bool & enabled, E_LevelMode & lvlmode) const;
	bool setLvlMode (QString const & item, bool enabled, E_LevelMode lvlmode);

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("enabled", m_enabled);
		ar & boost::serialization::make_nvp("lvl_filters", m_data);
	}

	typedef QList<FilteredLevel> lvl_filters_t;
	lvl_filters_t			m_data;
	QStandardItemModel *	m_model;
	QStyledItemDelegate *   m_delegate;

	Q_OBJECT
};

struct LevelDelegate : public QStyledItemDelegate
{
    LevelDelegate (QObject * parent = 0) : QStyledItemDelegate(parent) { }
    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

