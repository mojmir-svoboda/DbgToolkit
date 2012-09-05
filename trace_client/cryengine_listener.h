#pragma once
#include <CryCommon/platform.h>
#include <CryCommon/ILog.h>
#include "trace.h"

class EngineLogCallback : public ILogCallback
{
	virtual TRACE_API void OnWriteToConsole (const char * sText, bool bNewLine);
	virtual TRACE_API void OnWriteToFile (const char * sText, bool bNewLine);
};
