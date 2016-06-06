#pragma once
#include <QString>
#include <QColor>
#include <QVector>
#include <QMetaType>
#include <QRegularExpression>

struct CheckedComboBoxConfig
{
	std::vector<QString> m_base;
	std::vector<int> m_states; // checked or unchecked
	std::vector<QString> m_combined_states; // each entry in form "str1|str2|..|strN"
	QString m_current;

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("base", m_base);
		ar & boost::serialization::make_nvp("states", m_states);
		ar & boost::serialization::make_nvp("combined_states", m_combined_states);
		ar & boost::serialization::make_nvp("current", m_current);
	}

	void clear ()
	{
		m_base.clear();
		m_states.clear();
		m_combined_states.clear();
		m_current.clear();
	}

	bool hasValidConfig () const
	{
		return !m_current.isEmpty();
	}
};

