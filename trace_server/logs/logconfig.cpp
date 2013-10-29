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
}


