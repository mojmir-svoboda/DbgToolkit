#pragma once

//#define WH_API_FRAMEWORK

#define DBG_OUT printf
//#define DBG_OUT(fmt, ...) ((void)0)

#define CACHE_LINE 64

#if defined _MSC_VER
#	define CACHE_ALIGN __declspec(align(CACHE_LINE))
#else
#  define CACHE_ALIGN __attribute__((aligned(CACHE_LINE)))
#endif

#   define WH_PROFILER_APPNAME PROFILE_APPNAME
#   define WH_PROFILER_CONNECT PROFILE_CONNECT
#   define WH_PROFILER_DISCONNECT PROFILE_DISCONNECT

#   define WH_PROFILE_BEGIN PROFILE_BGN
#   define WH_PROFILE_END PROFILE_END
#   define WH_PROFILE_FRAME_BGN PROFILE_FRAME_BGN
#   define WH_PROFILE_FRAME_END PROFILE_FRAME_END
#   define WH_PROFILE_SCOPE PROFILE_SCOPE

