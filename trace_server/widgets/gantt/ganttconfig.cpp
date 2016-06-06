#include "ganttconfig.h"
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <fstream>
#include <sstream>

namespace gantt {

	bool loadConfig (GanttConfig & config, QString const & fname)
	{
		std::ifstream ifs(fname.toLatin1());
		if (!ifs) return false;
		boost::archive::xml_iarchive ia(ifs);
		ia >> BOOST_SERIALIZATION_NVP(config);
		ifs.close();
		return true;
	}

	bool saveConfig (GanttConfig const & config, QString const & fname)
	{
		std::ofstream ofs(fname.toLatin1());
		if (!ofs) return false;
		boost::archive::xml_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(config);
		ofs.close();
		return true;
	}

	void fillDefaultConfig (GanttConfig & config)
	{
		config = GanttConfig();
	}
}


