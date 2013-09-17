#pragma once
#include "filterbase.h"
#include "ui_filter_ctx.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStyledItemDelegate>
#include "appdata.h"

struct FilterCtx : FilterBase
{
	Ui_FilterCtx * m_ui;

	FilterCtx (QWidget * parent = 0);
	virtual ~FilterCtx ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Ctx; }

	virtual bool accept (DecodedCommand const & cmd) const;

	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();
	virtual void clear ();

	// ctx specific
	void setupModel ();
	void detroyModel ();
	bool isCtxPresent (QString const & item, bool & enabled) const;
	void appendCtxFilter (QString const & item);
	void removeCtxFilter (QString const & item);

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("ctx_filters", m_ctx_filters);
	}

	typedef QList<FilteredContext> ctx_filters_t;
	ctx_filters_t			m_ctx_filters;
	QStandardItemModel *	m_ctx_model;
	QStyledItemDelegate *   m_delegate;

	Q_OBJECT
};

struct CtxDelegate : public QStyledItemDelegate
{
	AppData const & m_app_data;
    CtxDelegate (AppData const & ad, QObject *parent = 0) : QStyledItemDelegate(parent), m_app_data(ad) { }
    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};


