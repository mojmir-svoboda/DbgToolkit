#include "dockconfig.h"
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <serialize/ser_qstringlist.h>
#include <fstream>
#include <sstream>
#include "serialize.h"

bool loadConfig (DockConfig & config, QString const & fname)
{
    return loadConfigTemplate(config, fname);
}

bool saveConfig (DockConfig const & config, QString const & fname)
{
    return saveConfigTemplate(config, fname);
}

void fillDefaultConfig (DockConfig & config)
{
	config.defaultConfig();
}


