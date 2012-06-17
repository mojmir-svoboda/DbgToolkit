#include "serialization.h"
#include "sessionstate.h"
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

bool saveSessionState (SessionState const & s, char const * file)
{
    std::ofstream ofs(file);
    if (!ofs) return false;
    boost::archive::text_oarchive oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(s);
    ofs.close();
    return true;
}

bool loadSessionState (SessionState & s, char const * file)
{
    std::ifstream ifs(file);
    if (!ifs) return false;
    boost::archive::text_iarchive ia(ifs);
    ia >> BOOST_SERIALIZATION_NVP(s);
    ifs.close();

	return true;
}

bool loadSessionState (SessionState const & src, SessionState & target)
{
    std::stringstream s;
    boost::archive::text_oarchive oa(s);
    oa << BOOST_SERIALIZATION_NVP(src);

    boost::archive::text_iarchive ia(s);
    ia >> BOOST_SERIALIZATION_NVP(target);
    return true;
}
