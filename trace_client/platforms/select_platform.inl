#pragma once

#if defined WIN32
#	if defined TRACE_WINDOWS_USES_SOCKET
#		include "trace_win_socket.inl"
#	elif defined TRACE_WINDOWS_USES_MEMMAP || defined TRACE_WINDOWS_USES_FILE
#		include "trace_win_file.inl"
#	endif

#elif defined __linux__
#	include "trace_linux_socket.inl"

#elif defined _XBOX
//#	if defined TRACE_XBOX360_USES_SOCKET
#		include "trace_x360_socket.inl"
//#	elif defined TRACE_XBOX360_USES_FILE
//#		include "trace_xbox360_file.inl"
//#	endif

#elif defined _PS3
#	if defined TRACE_PS3_USES_SOCKET
#		include "trace_ps3_socket.inl"
#	endif
#endif

#undef CACHE_ALIGN
#undef CACHE_LINE
