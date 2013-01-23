#include "config.h"
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include "serialize/ser_qlist.h"
#include "serialize/ser_qcolor.h"
#include "serialize/ser_qregexp.h"
#include "serialize/ser_qstring.h"
#include <fstream>
#include <sstream>

bool saveHistory (History<QString> const & h, char const * filename)
{
    std::ofstream ofs(filename);
    if (!ofs) return false;
    boost::archive::xml_oarchive oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(h);
    ofs.close();
    return true;
}

bool loadHistory (History<QString> & h, char const * filename)
{
    std::ifstream ifs(filename);
    if (!ifs) return false;
    boost::archive::xml_iarchive ia(ifs);
    ia >> BOOST_SERIALIZATION_NVP(h);
    ifs.close();
	return true;
}

void GlobalConfig::loadSearchHistory ()
{
	QString const filename = m_appdir + "/search_history.xml";
	loadHistory(m_search_history, filename.toLatin1());
}

void GlobalConfig::saveSearchHistory () const
{
	QString const filename = m_appdir + "/search_history.xml";
	saveHistory(m_search_history, filename.toLatin1());
}


