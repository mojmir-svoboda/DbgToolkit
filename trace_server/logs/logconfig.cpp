#include "logconfig.h"
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <serialize.h>
#include <fstream>
#include <sstream>
#include <tlv_parser/tlv.h>

namespace logs {

	bool loadConfig (LogConfig & config, QString const & fname)
	{
		if (!::loadConfigTemplate(config, fname))
		{
			fillDefaultConfig(config);
			return false;
		}
		return true;
	}

	bool saveConfig (LogConfig const & config, QString const & fname)
	{
		return ::saveConfigTemplate(config, fname);
	}

	void fillDefaultConfig (LogConfig & config)
	{
		config = LogConfig();
	}

	bool validateConfig (logs::LogConfig const & cfg)
	{
		bool b = true;
		b &= cfg.m_columns_setup.size() != 0;
		b &= cfg.m_columns_sizes.size() != 0;
		b &= cfg.m_columns_align.size() != 0;
		b &= cfg.m_columns_elide.size() != 0;

		if (!b)
			return false;

		bool same = cfg.m_columns_setup.size() == cfg.m_columns_sizes.size();
		same &= cfg.m_columns_sizes.size() == cfg.m_columns_align.size();
		same &= cfg.m_columns_align.size() == cfg.m_columns_elide.size();

		bool order_sane = true;
		for (int i = 0, ie = cfg.m_columns_setup.size(); i < ie; ++i)
		{
			QString const & item = cfg.m_columns_setup.at(0);
			if (!item.isEmpty() && tlv::tag_for_name(item.toStdString().c_str()) != 0)
				order_sane &= true;
			else
				order_sane &= false;
		}

		return same;
	}
}


