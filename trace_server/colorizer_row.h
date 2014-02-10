#pragma once
#include "filterbase.h"
#include "ui_colorizer_row.h"
#include <boost/serialization/nvp.hpp>
#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include <logs/logtablemodel.h>

struct ColorizedRow {
	QColor	m_fgcolor;
	QColor	m_bgcolor;
	int m_row;
	QString m_row_str;
	bool	m_is_enabled;

	ColorizedRow () { }

	/*ColorizedRow (QString const & rs)
        , m_fgcolor(Qt::blue), m_bgcolor(Qt::white), m_row_str(rs), m_row(rs), m_is_enabled(0)
	{ }*/

	ColorizedRow (QString const & rs, QColor const & col, QColor const & bgcol)
        : m_fgcolor(col), m_bgcolor(bgcol), m_row_str(rs), m_row(rs.toInt()), m_is_enabled(0)
	{ }

	ColorizedRow (int row, QColor const & col, QColor const & bgcol)
        : m_row_str(QString::number(row)), m_row(row), m_is_enabled(0)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("row_str", m_row_str);
		ar & boost::serialization::make_nvp("fgcolor", m_fgcolor);
		ar & boost::serialization::make_nvp("bgcolor", m_bgcolor);
		ar & boost::serialization::make_nvp("row", m_row);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
	}
};


struct ColorizerRow : FilterBase
{
	Ui_ColorizerRow * m_ui;

	ColorizerRow (QWidget * parent = 0);
	virtual ~ColorizerRow ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Colorizer_Row; }

	virtual bool accept (DecodedCommand const & cmd) const;
	virtual bool action (DecodedCommand const & cmd);

	virtual void defaultConfig ();
	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();
	virtual void clear ();

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterBase::serialize(ar, version);
		ar & boost::serialization::make_nvp("colorizer_regex", m_data);
	}

	// color_regex specific
	ColorizedRow & findOrCreateColorizedRow (QString const & str);
	void setupModel ();
	void destroyModel ();
	ColorizedRow const * findMatch (QString const & item) const;
	void append (QString const & item);
	void remove (QString const & item);
	void recompile ();
	void setConfigToUI ();

	void colorize (QString const & row_str, QColor const & fg, QColor const & bg);
	ColorizedRow & add (QString const & row_str, QColor const & fg, QColor const & bg);
	//void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }
	void onColorRowChanged (int role);
	bool isRowColored (int row) const;

	void actionColorRow (DecodedCommand const & cmd, ColorizedRow const & ct) const;
	void actionUncolorRow (DecodedCommand const & cmd, ColorizedRow const & ct) const;
	void updateColorRow (ColorizedRow const & ct);
	void recompileColorRow (ColorizedRow & ct);
	void recompileColorRows ();
	void uncolorRow (ColorizedRow const & ct);

	void setSrcModel (LogTableModel * m) { m_src_model = m; }

	typedef QList<ColorizedRow> filters_t;
	filters_t				m_data;
	QStandardItemModel *	m_model;
	LogTableModel *			m_src_model; // @FIXME: not happy about this, but i need it fast :/

	Q_OBJECT
public slots:
	void onClickedAt (QModelIndex idx);
	void onSelectAll ();
	void onSelectNone ();
	void onAdd ();
	void onRm ();
	void onActivate (int);
		//void onColorRowAdd ();
		//void onColorRowRm ();
	void onFgChanged ();
	void onBgChanged ();

signals:
};

struct ColorizerRowDelegate : public QStyledItemDelegate
{
	ColorizerRowDelegate (QObject * parent = 0) : QStyledItemDelegate(parent) { }
	~ColorizerRowDelegate ();
	void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

