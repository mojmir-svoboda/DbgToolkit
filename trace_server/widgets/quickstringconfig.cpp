#include "quickstringconfig.h"
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <serialize/serialize.h>
#include <fstream>
#include <sstream>

bool loadConfig (QuickStringConfig & config, QString const & fname)
{
	if (!::loadConfigTemplate(config, fname))
		fillDefaultConfig(config);
	return true;
}

bool saveConfig (QuickStringConfig const & config, QString const & fname)
{
	return ::saveConfigTemplate(config, fname);
}

void QuickStringConfig::clear ()
{
	fillDefaultConfig(*this);
}

void fillDefaultConfig (QuickStringConfig & config)
{
	config = QuickStringConfig();
}


