#pragma once

#if defined	WIN32 || defined WIN64

# define TRACE_CLIENT_SINKS FileClient<typelist<LogISOTime, I, LogShortFile, Separator<':'>, LogLine, I, LogFunc, I>>
//LogTime, I, LogLevel, I, LogContext, I, LogFile, Separator<':'>, LogLine, I, LogFunc, I
//LogTime, I, LogFunc, I, LogContext, I, LogFile, Separator<':'>, LogLine, I, LogLevel, I
//LogISOTime, I

#endif

