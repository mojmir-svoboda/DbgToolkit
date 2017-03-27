#pragma once

#if defined	WIN32 || defined WIN64

# define TRACE_CLIENT_SINKS AsioSocketClient, FileClient<typelist<LogISOTime, I, LogFunc, Separator<':'>, LogLine, I>>
# define TRACE_FILE_CLIENT_FORMAT 

#endif

