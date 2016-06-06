#pragma once
#include <filters/filterbase.h>
#include "ui_colorizer_row.h"
#include <boost/serialization/nvp.hpp>
#include "config.h"
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include <widgets/logs/logtablemodel.h>

struct ColorizedRow {
	bool	m_is_enabled;
	QColor	m_fgcolor;
	QColor	m_bgcolor;
	int m_row;
	QString m_row_str;

	//bool accept (QString str) const {...}

	ColorizedRow () { }

	ColorizedRow (int row, QColor const & col, QColor const & bgcol)
        : m_fgcolor(col), m_bgcolor(bgcol), m_row_str(QString::number(row)), m_row(row), m_is_enabled(0)
	{ }

	ColorizedRow (QString const & row_str, QColor const & col, QColor const & bgcol)
        : m_fgcolor(col), m_bgcolor(bgcol), m_row_str(row_str), m_row(row_str.toInt()), m_is_enabled(0)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("fgcolor", m_fgcolor);
		ar & boost::serialization::make_nvp("bgcolor", m_bgcolor);
		ar & boost::serialization::make_nvp("row_str", m_row_str);
		ar & boost::serialization::make_nvp("row", m_row);
	}
};


struct ColorizerRow : FilterBase
{
	Ui_ColorizerRow * m_ui;

	ColorizerRow (QWidget * parent = 0);
	virtual ~ColorizerRow ();

	virtual void initUI () override;
	virtual void doneUI () override;

	virtual E_FilterType type () const override { return e_Colorizer_Row; }

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
		ar & boost::serialization::make_nvp("colorizer_row", m_data);
	}

	// color_regex specific
	void setSrcModel(BaseTableModel * m) { m_src_model = m; }
	ColorizedRow & findOrCreateColorizedRow (QString const & str);
	void setupModel ();
	void destroyModel ();
	ColorizedRow const * findMatch (QString const & item) const;
	void append (QString const & item);
	void remove (QString const & item);
	void recompile ();
	void setConfigToUI ();

	ColorizedRow & add (int row, QColor const & fg, QColor const & bg);
	//void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }
	void onColorButtonChanged (int role);

	void actionColor (QModelIndex const & sourceIndex, ColorizedRow const & ct, QColor const & fg, QColor const & bg) const;
	void actionUncolor (QModelIndex const & sourceIndex, ColorizedRow const & ct) const;
	void updateColor (ColorizedRow const & ct);
	void uncolor (ColorizedRow const & ct);
	void recompileColorRows ();

	bool isRowColored (int row) const;
	void recompileColorRow (ColorizedRow & ct);
	void colorize (int row, QColor const & fg, QColor const & bg);
	void uncolorize (QString const & row);

	typedef std::vector<ColorizedRow> filters_t;
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
