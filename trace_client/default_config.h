#pragma once

#if defined	(WIN32) || defined (WIN64)
#	define TRACE_WINDOWS_USES_SOCKET 1
#endif

#if defined (_XBOX)
#	define TRACE_XBOX360_USES_SOCKET 1
#endif
