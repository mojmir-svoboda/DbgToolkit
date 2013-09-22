#pragma once
#include "filterbase.h"
#include "ui_filter_ctx.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include "appdata.h"

struct CtxDelegate : public QStyledItemDelegate
{
	AppData const * m_app_data;
    CtxDelegate (QObject * parent = 0) : QStyledItemDelegate(parent), m_app_data(0) { }
    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;

	void setAppData (AppData const * appdata) { m_app_data = appdata; }
};


struct FilterCtx : FilterBase
{
	Ui_FilterCtx * m_ui;

	FilterCtx (QWidget * parent = 0);
	virtual ~FilterCtx ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Ctx; }

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
		ar & boost::serialization::make_nvp("ctx_filters", m_data);
	}

	// ctx specific
	void setAppData (AppData const * appdata) { static_cast<CtxDelegate *>(m_delegate)->setAppData(appdata); }
	void setupModel ();
	void destroyModel ();
	bool isCtxPresent (QString const & item, bool & enabled) const;
	void appendCtxFilter (QString const & item);
	void removeCtxFilter (QString const & item);
	void recompile ();
	void setConfigToUI ();
	void appendToCtxWidgets (FilteredContext const & flt);

	typedef QList<FilteredContext> ctx_filters_t;
	ctx_filters_t			m_data;
	QStandardItemModel *	m_model;
	QStyledItemDelegate *   m_delegate;

	Q_OBJECT
public slots:
	void onClickedAtCtxTree (QModelIndex idx);
	void onSelectAllCtxs ();
	void onSelectNoCtxs ();
signals:
};

