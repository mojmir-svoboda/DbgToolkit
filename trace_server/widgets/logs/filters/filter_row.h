#pragma once
#include <filters/filterbase.h>
#include "ui_filter_row.h"

#include "config.h"
#include "utils_filters.h"
#include <QObject>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

namespace logs {

struct FilteredRow {
	int m_row;
	QString m_row_str;
	bool m_is_enabled;
	int m_operator;
	QString m_operator_str;

	FilteredRow () { }
	FilteredRow (int row, bool enabled, E_CmpMode op)
        : m_row(row), m_row_str(QString::number(row)), m_is_enabled(enabled), m_operator(op), m_operator_str(cmpModToString(op))
	{ }

	bool isValid () const { return true; }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("row", m_row);
		ar & boost::serialization::make_nvp("row_str", m_row_str);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("operator", m_operator);
		ar & boost::serialization::make_nvp("operator_str", m_operator_str);
	}

	bool accept (int n) const
	{
		if (!m_is_enabled)
			return true;

		bool const res = int_comparators[m_operator](n, m_row);
		return res;
	}
};

inline bool operator< (FilteredRow const & lhs, FilteredRow const & rhs)
{
	return lhs.m_row < rhs.m_row;
}

struct FilterRow : FilterBase
{
	Ui_FilterRow * m_ui;

	FilterRow (QWidget * parent = 0);
	virtual ~FilterRow ();

	virtual void initUI () override;
	virtual void doneUI () override;

	virtual E_FilterType type () const override { return e_Filter_Row; }

	virtual bool accept (QModelIndex const & sourceIndex) override;

	virtual void defaultConfig () override;
	virtual void loadConfig (QString const & path) override;
	virtual void saveConfig (QString const & path) override;
	virtual void applyConfig () override;
	virtual void clear () override;

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
// 		FilterBase::serialize(ar, version);
// 		ar & boost::serialization::make_nvp("row_filters", m_data);
	}

	// row specific
	void setupModel ();
	void destroyModel ();
	FilteredRow & findOrCreateFilteredRow (QString const & item);
	void setConfigToUI ();
	void locateItem (QString const & item, bool scrollto, bool expand);
	void addFilteredRow (FilteredRow const & cfg);
	void addRowToUI (FilteredRow const & cfg);
	void removeFromConfig (QString const & s);
	void setupModelHeader ();

	typedef std::vector<FilteredRow> row_filters_t;
	row_filters_t			m_data;
	QStandardItemModel *	m_model;
	QStyledItemDelegate *   m_delegate;

	Q_OBJECT
public slots:
	void onSelectAll ();
	void onSelectNone ();
	void onRm ();
	void onAdd ();
	void onDataChanged (QModelIndex const &, QModelIndex const &);
	void onOperatorChanged (int);
signals:
};

}
