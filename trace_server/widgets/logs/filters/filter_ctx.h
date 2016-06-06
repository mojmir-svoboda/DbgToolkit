#pragma once
#include <filters/filterbase.h>
#include "ui_filter_ctx.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include "appdata.h"

namespace logs {

struct FilteredContext
{
	QString m_ctx_str;
	uint64_t m_ctx;
	bool m_is_enabled;
	int m_state;

	FilteredContext () { }
	FilteredContext (uint64_t ctx, bool enabled, int state)
        : m_ctx_str(QString::number(ctx)), m_ctx(ctx), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("ctx_str", m_ctx_str);
		ar & boost::serialization::make_nvp("ctx", m_ctx);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

inline bool operator< (FilteredContext const & lhs, FilteredContext const & rhs)
{
	return lhs.m_ctx < rhs.m_ctx;
}

struct FilterCtx : FilterBase
{
	Ui_FilterCtx * m_ui;

	FilterCtx (QWidget * parent = 0);
	virtual ~FilterCtx ();

	virtual void initUI () override;
	virtual void doneUI () override;

	virtual E_FilterType type () const override { return e_Filter_Ctx; }

	virtual bool accept (QModelIndex const & sourceIndex) override;

	virtual void defaultConfig () override;
	virtual void loadConfig (QString const & path) override;
	virtual void saveConfig (QString const & path) override;
	virtual void applyConfig () override;
	virtual void clear () override;

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterBase::serialize(ar, version);
		ar & boost::serialization::make_nvp("ctx_filters", m_data);
	}

	// ctx specific
	void setAppData (AppData const * appdata);
	void setupModel ();
	void destroyModel ();
	bool isCtxPresent (uint64_t ctx, bool & enabled) const;
	void appendCtxFilter (uint64_t ctx);
	void createEntriesFor (uint64_t ctx);
	void removeCtxFilter (uint64_t ctx);
	void recompile ();
	void setConfigToUI ();
	void appendToCtxWidgets (FilteredContext const & flt);
	void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }

	typedef std::vector<FilteredContext> ctx_filters_t;
	ctx_filters_t			m_data;
	QStandardItemModel *	m_model;

	Q_OBJECT
public slots:
	void onClickedAtCtx (QModelIndex idx);
	void onSelectAll ();
	void onSelectNone ();
	void onDictionaryArrived (int type, Dict const & d);
signals:
};

struct CtxDelegate : public QStyledItemDelegate
{
	AppData const * m_app_data;
	CtxDelegate (QObject * parent = 0) : QStyledItemDelegate(parent), m_app_data(0) { }
	~CtxDelegate ();
	void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;

	void setAppData (AppData const * appdata) { m_app_data = appdata; }
};

}
