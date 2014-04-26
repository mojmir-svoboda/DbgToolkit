#include "connectionconfig.h"
#include "serialize.h"

inline bool saveHistoryState (History<QString> const & h, char const * filename)
{
	//return ::saveConfigTemplate(h, filename);
}

inline bool loadHistoryState (History<QString> & h, char const * filename)
{
	//::loadConfigTemplate(h, filename);
	return true;
}

void ConnectionConfig::loadHistory ()
{
	//QString const pfilename = m_appdir + "/preset_history.xml";
	//loadHistoryState(m_preset_history, pfilename.toLatin1());
}

void ConnectionConfig::saveHistory () const
{
	//QString const pfilename = m_appdir + "/preset_history.xml";
	//saveHistoryState(m_preset_history, pfilename.toLatin1());
}


