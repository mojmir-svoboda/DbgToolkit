#pragma once
#include <filters/filterbase.h>
#include "ui_filter_script.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

struct FilteredScript {
	QString m_name;
	QString m_script_str;
	QString m_path;
	bool m_is_enabled;
	int m_state;

	//bool match (QString const & str) const { return str.contains(m_string, Qt::CaseInsensitive); }

	FilteredScript () { }
	FilteredScript (QString const & name, bool enabled, int state)
		: m_name(name), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("name", m_name);
		ar & boost::serialization::make_nvp("path", m_path);
		ar & boost::serialization::make_nvp("script", m_script_str);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

class QScriptEngine;

struct FilterScript : FilterBase
{
	Ui_FilterScript * m_ui;

	FilterScript (QWidget * parent = 0);
	virtual ~FilterScript ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Script; }

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
		ar & boost::serialization::make_nvp("filtered_strings", m_data);
	}

	// string filtering
	void setupModel ();
	void destroyModel ();
	void appendToScriptFilters (QString const & str, bool checked, int state);
	void appendToScriptWidgets (FilteredScript const & flt);
	void removeFromScriptFilters (QString const & str);
	//bool isMatchedScriptExcluded (QString str) const;
	void setScriptChecked (QString const & s, bool checked);
	void setScriptState (QString const & s, int state);
	void recompile ();
	void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }

	QList<FilteredScript>	m_data;
	QScriptEngine *			m_se;
	QStandardItemModel *	m_model;
	QStyledItemDelegate *   m_delegate;

	Q_OBJECT
public slots:
	void onClickedAtScriptList (QModelIndex idx);
	void onScriptRm ();
	void onScriptAdd ();
signals:

};

struct ScriptDelegate : public QStyledItemDelegate
{
	ScriptDelegate (QObject * parent = 0) : QStyledItemDelegate(parent) { }
	void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

