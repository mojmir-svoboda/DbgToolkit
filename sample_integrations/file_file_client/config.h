#pragma once

#if defined	WIN32 || defined WIN64

# define TRACE_CLIENT_SINKS FileClient<typelist<LogTime, I, LogFunc, I, LogContext, I, LogFile, Separator<':'>, LogLine, I, LogLevel, I>>, FileClient<typelist<LogISOTime, I, LogFunc, Separator<':'>, LogLine, I>>

#endif

