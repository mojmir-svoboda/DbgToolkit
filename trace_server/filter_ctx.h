#pragma once
#include "filterbase.h"
#include "ui_filter_ctx.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>

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

	Q_OBJECT

	// ctx
	typedef QList<FilteredContext> ctx_filters_t;
	bool isCtxPresent (QString const & item, bool & enabled) const;
	void appendCtxFilter (QString const & item);
	void removeCtxFilter (QString const & item);
	void onClearCtxFilter () { m_ctx_filters.clear(); }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("ctx_filters", m_ctx_filters);
	}
	ctx_filters_t			m_ctx_filters;


};
