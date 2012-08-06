#include "config.h"
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include "serialize/ser_qlist.h"
#include "serialize/ser_qcolor.h"
#include "serialize/ser_qregexp.h"
#include "serialize/ser_qstring.h"
#include <fstream>
#include <sstream>

namespace plot {

	bool loadConfig (PlotConfig & config, QString const & fname)
	{
		std::ifstream ifs(fname.toAscii());
		if (!ifs) return false;
		boost::archive::text_iarchive ia(ifs);
		ia >> BOOST_SERIALIZATION_NVP(config);
		ifs.close();
		return true;
	}

	bool saveConfig (PlotConfig const & config, QString const & fname)
	{
		std::ofstream ofs(fname.toAscii());
		if (!ofs) return false;
		boost::archive::text_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(config);
		ofs.close();
		return true;
	}

}
