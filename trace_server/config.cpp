#include "config.h"
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <serialize/ser_qlist.h>
#include <serialize/ser_qcolor.h>
#include <serialize/ser_qregexp.h>
#include <serialize/ser_qstring.h>
#include <serialize.h>
#include <fstream>
#include <sstream>

bool saveHistoryState (History<QString> const & h, char const * filename)
{
	return ::saveConfigTemplate(h, filename);
}

bool loadHistoryState (History<QString> & h, char const * filename)
{
	::loadConfigTemplate(h, filename);
	return true;
}

void GlobalConfig::loadHistory ()
{
	QString const sfilename = m_appdir + "/search_history.xml";
	loadHistoryState(m_search_history, sfilename.toLatin1());
	QString const pfilename = m_appdir + "/preset_history.xml";
	loadHistoryState(m_preset_history, pfilename.toLatin1());
}

void GlobalConfig::saveHistory () const
{
	QString const sfilename = m_appdir + "/search_history.xml";
	QString const pfilename = m_appdir + "/preset_history.xml";
	saveHistoryState(m_search_history, sfilename.toLatin1());
	saveHistoryState(m_preset_history, pfilename.toLatin1());
}


