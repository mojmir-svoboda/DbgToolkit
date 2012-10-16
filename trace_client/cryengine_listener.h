#pragma once
#include <CryCommon/platform.h>
#include <CryCommon/ILog.h>
#include "trace.h"

class EngineLogCallback : public ILogCallback
{
	virtual TRACE_API void OnWriteToConsole (const char * msg, bool);
	virtual TRACE_API void OnWriteToConsole (char const * file, int line, const char * msg, bool);
	virtual TRACE_API void OnWriteToFile (const char * msg, bool);
};
