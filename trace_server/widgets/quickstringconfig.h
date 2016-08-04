#pragma once
#include <QString>
#include <QColor>
#include <QVector>
#include <QMetaType>
#include <QRegularExpression>
#include <utils/history.h>
#include "checkedcomboboxconfig.h"

struct QuickStringConfig
{
	bool m_whole_word { false };
	bool m_case_sensitive { false };
	bool m_regexp { false };
	unsigned m_history_ln { 32 };
	History<QString> m_history;
	CheckedComboBoxConfig m_where;
	QString m_str;
	QRegularExpression m_regexp_val;

	QuickStringConfig ()
		: m_history(m_history_ln)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("whole_word", m_whole_word);
		ar & boost::serialization::make_nvp("case_sensitive", m_case_sensitive);
		ar & boost::serialization::make_nvp("regexp", m_regexp);
		ar & boost::serialization::make_nvp("history_ln", m_history_ln);
		ar & boost::serialization::make_nvp("history", m_history);
		ar & boost::serialization::make_nvp("where", m_where);
		ar & boost::serialization::make_nvp("str", m_str);
	}

	void clear ();
};

Q_DECLARE_METATYPE(QuickStringConfig);

inline bool matchToQuickStringConfig (QString const & str, QuickStringConfig const & fc)
{
	if (fc.m_regexp)
	{
		QRegularExpression const & r = fc.m_regexp_val;
		if (r.isValid())
		{
			QRegularExpressionMatch m = r.match(str);
			return m.hasMatch();
		}
	}
	else
	{
		Qt::CaseSensitivity const cs = fc.m_case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
		if (str.contains(fc.m_str, cs))
		{
			return true;
		}
	}
	return false;
}

bool loadConfig (QuickStringConfig & config, QString const & fname);
bool saveConfig (QuickStringConfig const & config, QString const & fname);
void fillDefaultConfig (QuickStringConfig & config);

