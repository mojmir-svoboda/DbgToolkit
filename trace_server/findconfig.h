#pragma once
#include <QString>
#include <QColor>
#include <QVector>
#include "history.h"

struct FindConfig
{
	bool m_whole_word;
	bool m_case_sensitive;
	bool m_regexp;
	bool m_refs;
	bool m_clone;
	unsigned m_history_ln;
	History<QString> m_history;
	QString m_to_widget;
	QString m_str;

	FindConfig ()
		: m_whole_word(false)
		, m_case_sensitive(false)
		, m_regexp(false)
		, m_refs(true)
		, m_clone(false)
		, m_history_ln(32)
		, m_history(m_history_ln)
	{
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("whole_word", m_whole_word);
		ar & boost::serialization::make_nvp("case_sensitive", m_case_sensitive);
		ar & boost::serialization::make_nvp("regexp", m_regexp);
		ar & boost::serialization::make_nvp("refs", m_refs);
		ar & boost::serialization::make_nvp("clone", m_clone);
		ar & boost::serialization::make_nvp("history_ln", m_history_ln);
		ar & boost::serialization::make_nvp("history", m_history);
		ar & boost::serialization::make_nvp("to_widget", m_to_widget);
		ar & boost::serialization::make_nvp("str", m_str);
	}

	void clear ();
};

Q_DECLARE_METATYPE(FindConfig)

bool loadConfig (FindConfig & config, QString const & fname);
bool saveConfig (FindConfig const & config, QString const & fname);
void fillDefaultConfig (FindConfig & config);

