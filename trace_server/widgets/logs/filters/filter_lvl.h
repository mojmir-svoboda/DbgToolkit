#pragma once
#include <filters/filterbase.h>
#include "ui_filter_lvl.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include "appdata.h"

namespace logs {

struct FilteredLevel {
	QString m_level_str;
	uint64_t m_level;
	bool m_is_enabled;
	unsigned m_state;

	FilteredLevel () { }
	FilteredLevel (uint64_t level, bool enabled, E_LevelMode state)
        : m_level_str(QString::number(level)), m_level(level), m_is_enabled(enabled), m_state(static_cast<unsigned>(state))
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("level_str", m_level_str);
		ar & boost::serialization::make_nvp("level", m_level);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

inline bool operator< (FilteredLevel const & lhs, FilteredLevel const & rhs)
{
	return lhs.m_level < rhs.m_level;
}


struct FilterLvl : FilterBase
{
	Ui_FilterLvl * m_ui;

	FilterLvl (QWidget * parent = 0);
	virtual ~FilterLvl ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Lvl; }

	virtual bool accept (QModelIndex const & sourceIndex);

	virtual void defaultConfig ();
	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();
	virtual void clear ();

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterBase::serialize(ar, version);
		ar & boost::serialization::make_nvp("lvl_filters", m_data);
	}
	
	void setAppData (AppData const * appdata);

	// lvl specific
	void setupModel ();
	void destroyModel ();
	bool isPresent (uint64_t item, bool & enabled, E_LevelMode & lvlmode) const;
	void append (uint64_t item);
	void createEntriesFor (uint64_t item);
	void remove (uint64_t item);
	bool setMode (uint64_t item, bool enabled, E_LevelMode lvlmode);
	void recompile ();
	void setConfigToUI ();
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }
	void locateItem (QString const & item, bool scrollto, bool expand);

	typedef std::vector<FilteredLevel> lvl_filters_t;
	lvl_filters_t			m_data;
	QStandardItemModel *	m_model;
	QStyledItemDelegate *   m_delegate;

	Q_OBJECT
public slots:
	void onClicked (QModelIndex idx);
	void onSelectAll ();
	void onSelectNone ();
signals:
};

struct LevelDelegate : public QStyledItemDelegate
{
	AppData const * m_app_data;
	LevelDelegate (QObject * parent = 0) : QStyledItemDelegate(parent), m_app_data(nullptr) { }
	void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
	void setAppData (AppData const * appdata) { m_app_data = appdata; }
};

}