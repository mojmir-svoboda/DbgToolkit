#pragma once
#include <filters/filterbase.h>
#include "ui_colorizer_regex.h"
#include <boost/serialization/nvp.hpp>
#include "config.h"
#include <vector>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include <widgets/logs/logtablemodel.h>

struct ColorizedText {
	bool	m_is_enabled;
	QColor	m_fgcolor;
	QColor	m_bgcolor;
	QString m_regex_str;
	QRegularExpression m_regex;

	bool isValid () const { return m_regex.isValid(); }

	bool accept (QString str) const
	{
		if (m_is_enabled)
			if (m_regex.isValid())
			{
				QRegularExpressionMatch m = m_regex.match(str);
				return m.hasMatch();
			}
		return false;
	}

	ColorizedText () { }

	ColorizedText (QString const & rs, QColor const & col, QColor const & bgcol)
        : m_fgcolor(col), m_bgcolor(bgcol), m_regex_str(rs), m_regex(rs), m_is_enabled(0)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("fgcolor", m_fgcolor);
		ar & boost::serialization::make_nvp("bgcolor", m_bgcolor);
		ar & boost::serialization::make_nvp("regex_str", m_regex_str);
		ar & boost::serialization::make_nvp("regex", m_regex);
	}
};


struct ColorizerRegex : FilterBase
{
	Ui_ColorizerRegex * m_ui;

	ColorizerRegex (QWidget * parent = 0);
	virtual ~ColorizerRegex ();

	virtual void initUI () override;
	virtual void doneUI () override;

	virtual E_FilterType type () const override { return e_Colorizer_Regex; }

/*	virtual bool accept (QModelIndex const & sourceIndex) const override;*/
	virtual bool action (QModelIndex const & sourceIndex) override;

	virtual void defaultConfig () override;
	virtual void loadConfig (QString const & path) override;
	virtual void saveConfig (QString const & path) override;
	virtual void applyConfig () override;
	virtual void clear () override;

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterBase::serialize(ar, version);
		ar & boost::serialization::make_nvp("colorizer_regex", m_data);
	}

	// color_regex specific
	ColorizedText & findOrCreateColorizedText (QString const & str);
	void setupModel ();
	void setSrcModel (BaseTableModel * m) { m_src_model = m; }
	void destroyModel ();
	ColorizedText const * findMatch (QString const & item) const;
	void append (QString const & item);
	void remove (QString const & item);
	void recompile ();
	void setConfigToUI ();

	ColorizedText & add (QString const & regex, QColor const & fg, QColor const & bg);
	//void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }
	void onColorButtonChanged (int role);

	void actionColor (QModelIndex const & sourceIndex, ColorizedText const & ct, QColor const & fg, QColor const & bg) const;
	void actionUncolor (QModelIndex const & sourceIndex, ColorizedText const & ct) const;
	void updateColor (ColorizedText const & ct);
	void uncolor (ColorizedText const & ct);
	void recompileColorRegexps ();

	void recompileColorRegex (ColorizedText & ct);



	typedef std::vector<ColorizedText> filters_t;
	filters_t				m_data;
	QStandardItemModel *	m_model;
	BaseTableModel *			m_src_model; // @FIXME: not happy about this, but i need it fast :/

	Q_OBJECT
public slots:
	void onClickedAt (QModelIndex idx);
	void onSelectAll ();
	void onSelectNone ();
	void onAdd ();
	void onRm ();
	void onActivate (int);
	void onFgChanged ();
	void onBgChanged ();

signals:
};
