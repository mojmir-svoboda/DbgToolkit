#include "cryengine_listener.h"

#include <CryCommon/platform.h>
#include <CryCommon/platform_impl.h>
#include <CryCommon/ILog.h>

using namespace trace;

#if defined TRACE_ENABLED
namespace {
	struct CallbackSplitInfo
	{
		char const *	m_Substring;
		E_TraceLevel	m_TraceLevel;
	};
	// Here are rules for assigning trace level according to what is in string coming from CryEngine.
	// If a rule is valid, the next rules are not evaluated
	static CallbackSplitInfo s_CallbackSplitInfos[] =
	{
		{"Error parsing <", e_Error},
		{"[Error]", e_Error},
		{"[Lua Error]", e_Error},
		{"[Warning]", e_Warning},
	};
}
#endif

void EngineLogCallback::OnWriteToConsole (char const * msg, bool b)
{
	OnWriteToConsole(__FILE__, __LINE__, msg, b);
}

void EngineLogCallback::OnWriteToConsole (char const * file, int line, char const * msg, bool)
{
#if defined TRACE_ENABLED
	E_TraceLevel traceLevel = e_Info;
	for (int i = 0; i < sizeof(s_CallbackSplitInfos)/sizeof(CallbackSplitInfo); ++i)
	{
		if (strstr(msg, s_CallbackSplitInfos[i].m_Substring))
		{
			traceLevel = s_CallbackSplitInfos[i].m_TraceLevel;
			break;
		}
	}
	trace::Write(traceLevel, trace::CTX_Engine, file, line, __FUNCTION__, "%s", msg);
#else
	WH_UNUSED_VARIABLE(msg);
#endif
}

void EngineLogCallback::OnWriteToFile (char const * , bool)
{ }

