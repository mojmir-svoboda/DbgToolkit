#pragma once
#include "filterbase.h"
#include "ui_filter_script.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

class QScriptEngine;

struct FilterScript : FilterBase
{
	Ui_FilterScript * m_ui;

	FilterScript (QWidget * parent = 0);
	virtual ~FilterScript ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Script; }

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
	QStandardItemModel *	m_model;
	QStyledItemDelegate *   m_delegate;
	QScriptEngine *			m_se;

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

