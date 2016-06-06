#pragma once
#include "types.h"
#include "constants.h"
#include <array>

struct MixerConfig
{
	mixervalues_t m_state;
	std::vector<int> m_rows;
	std::vector<int> m_cols;
	bool m_default;

	MixerConfig ()
		: m_default(true)
	{
		m_state.fill(0);
		m_rows.resize(sizeof(level_t) * CHAR_BIT, -2);
		m_cols.resize(sizeof(context_t) * CHAR_BIT, -2);
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		m_default = false;
		ar & boost::serialization::make_nvp("state", m_default);
		ar & boost::serialization::make_nvp("state", m_state);
		ar & boost::serialization::make_nvp("rows", m_rows);
		ar & boost::serialization::make_nvp("cols", m_cols);
	}
};


