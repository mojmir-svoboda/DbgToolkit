#pragma once

#if defined WIN32
#	if defined PROFILE_WINDOWS_USES_SOCKET
#		include "profile_win_socket.inl"
#	elif defined PROFILE_WINDOWS_USES_MEMMAP || defined PROFILE_WINDOWS_USES_FILE
#		include "profile_win_file.inl"
#	endif

#elif defined __linux__
#	include "profile_linux_socket.inl"

#elif defined _XBOX
#	if defined PROFILE_XBOX360_USES_SOCKET
#		include "profile_xbox360_socket.inl"
#	elif defined PROFILE_XBOX360/USES_FILE
#		include "profile_xbox360_file.inl"
#	endif

#elif defined _PS3
#	if defined PROFILE_PS3_USES_SOCKET
#		include "profile_ps3_socket.inl"
#	endif
#endif

#undef CACHE_ALIGN
#undef CACHE_LINE
