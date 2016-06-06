#pragma once

struct WaveConfig
{
  QString m_name;
  QString m_fpath;

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("name", m_name);
		ar & boost::serialization::make_nvp("fpath", m_fpath);
	}
};

struct WaveTableConfig
{
	QString m_root;
	std::vector<WaveConfig> m_waves;

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("root", m_root);
		ar & boost::serialization::make_nvp("waves", m_waves);
	}

	void clear ()
	{
		m_waves.clear();
	}
};


