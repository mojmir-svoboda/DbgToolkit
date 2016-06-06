#include "dockmanagerconfig.h"
#include <serialize/serialize.h>

bool loadConfig (DockManagerConfig & config, QString const & fname)
{
    return loadConfigTemplate(config, fname);
}

bool saveConfig (DockManagerConfig const & config, QString const & fname)
{
    return saveConfigTemplate(config, fname);
}

