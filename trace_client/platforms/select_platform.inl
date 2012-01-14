#pragma once

#if defined WIN32
#	if defined TRACE_WINDOWS_USES_SOCKET
#		include "platforms/trace_win_socket.inl"
#	elif defined TRACE_WINDOWS_USES_MEMMAP
#		include "platforms/trace_win_file.inl"
#	elif defined TRACE_WINDOWS_USES_FILE
#		include "platforms/trace_win_file.inl"
#	endif

#elif defined _XBOX
#	if defined TRACE_XBOX360_USES_SOCKET
#		include "platforms/trace_xbox360_socket.inl"
#	elif defined TRACE_XBOX360/USES_FILE
#		include "platforms/trace_xbox360_file.inl"
#	endif

#elif defied _PS3
#	if defined TRACE_PS3_USES_SOCKET
#		include "platforms/trace_ps3_socket.inl"
#	endif
#endif

#undef CACHE_ALIGN
#undef CACHE_LINE
