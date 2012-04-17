#pragma once

#if defined WIN32 || defined WIN64

#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>

namespace wh {
	typedef LONGLONG timer_t;

	extern timer_t g_Start, g_Freq;

	inline timer_t queryPerformanceFrequency ()
	{
		LARGE_INTEGER tmp;
		QueryPerformanceFrequency(&tmp);
		timer_t const res = tmp.QuadPart;
		return res;
	}

	inline timer_t queryPerformanceCounter ()
	{
		LARGE_INTEGER tmp;
		QueryPerformanceCounter(&tmp);
		timer_t const res = tmp.QuadPart;
		return res;
	}

	void setTimeStart ();
	inline timer_t queryTime () { return queryPerformanceCounter() - g_Start; }
	inline double toSeconds (timer_t t) { return static_cast<double>(t) / g_Freq; }
}
#else

#include <sys/time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

namespace wh {
	static const unsigned usec_per_sec = 1000000;
	static const unsigned usec_per_msec = 1000;

	typedef int64_t timer_t;

	timer_t queryPerformanceFrequency ()
	{
		 /* gettimeofday reports to microsecond accuracy. */
		 return usec_per_sec;
	}

	timer_t queryPerformanceCounter ()
	{
		 struct timeval time;
		 gettimeofday(&time, NULL);
		 timer_t performance_count = time.tv_usec + /* Microseconds. */
									 time.tv_sec * usec_per_sec; /* Seconds. */
		 return performance_count;
	}
}

#endif

