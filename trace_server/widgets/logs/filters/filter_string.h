#pragma once
#include <filters/filterbase.h>
#include "ui_filter_string.h"
#include <boost/serialization/nvp.hpp>
#include "config.h"
#include <widgets/checkedcomboboxconfig.h>
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

namespace logs {

struct FilteredString
{
	bool m_is_enabled;
	bool m_negate_match;
	bool m_is_regex;
	bool m_case_sensitive;
	bool m_whole_word;
	QString m_regex_str;
	QRegularExpression m_regex;
	CheckedComboBoxConfig m_where;

	bool accept (QString const & str) const
	{
		if (m_is_enabled)
		{
			if (m_is_regex)
			{
				if (m_regex.isValid())
				{
					QRegularExpressionMatch m = m_regex.match(str);
					return m.hasMatch();
				}
				else
					return false;
			}
			else
			{
				bool const res = str.contains(m_regex_str, m_case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
				return res;
			}
		}
		return true;
	}
	bool isValid() const { return m_is_regex ? m_regex.isValid() : true; }

	FilteredString ()
		: FilteredString(false, false, false, true, false, QString(), CheckedComboBoxConfig())
	{ }
	FilteredString (bool on, bool neg, bool is_regex, bool cs, bool ww, QString const & rs, CheckedComboBoxConfig & cccfg)
		: m_is_enabled(on), m_negate_match(neg), m_is_regex(is_regex), m_case_sensitive(cs), m_whole_word(ww), m_regex_str(rs), m_regex(rs), m_where(cccfg)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("negate_match", m_negate_match);
		ar & boost::serialization::make_nvp("is_regex", m_is_regex);
		ar & boost::serialization::make_nvp("case_sensitive", m_case_sensitive);
		ar & boost::serialization::make_nvp("whole_word", m_whole_word);
		ar & boost::serialization::make_nvp("regex_str", m_regex_str);
		ar & boost::serialization::make_nvp("regex", m_regex);
		ar & boost::serialization::make_nvp("where", m_where);
	}
};


struct FilterString : FilterBase
{
	Ui_FilterString * m_ui;

	FilterString (QWidget * parent = 0);
	virtual ~FilterString ();

	virtual void initUI () override;
	virtual void doneUI () override;

	virtual E_FilterType type () const override { return e_Filter_String; }

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
		ar & boost::serialization::make_nvp("filtered_strings", m_data);
		ar & boost::serialization::make_nvp("where", m_cccfg);
	}

	// string filtering
	void setupModel ();
	void destroyModel ();
	FilteredString & findOrCreateFilteredString (QString const & item);
	void addRowToUI (FilteredString const & cfg);
	void removeFromStringFilters (QString const & str);
	void recompile ();
	void recompileRegex (FilteredString & cfg);
	void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }
	void setConfigToUI ();
	void setUIValuesToConfig (FilteredString & cfg);
	void setDefaultSearchConfig (CheckedComboBoxConfig const & cccfg);
	void setupModelHeader ();

	QList<FilteredString>	m_data;
	CheckedComboBoxConfig m_cccfg;
	QStandardItemModel *  m_model;

	Q_OBJECT
public slots:
	void onStringRm ();
	void onStringAdd ();
	void onDataChanged (QModelIndex const &, QModelIndex const &);
signals:

};

}
