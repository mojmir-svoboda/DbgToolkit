#pragma once
#define WIN32_LEAN_AND_MEAN
#if defined __MINGW32__
#   undef _WIN32_WINNT
#   define _WIN32_WINNT 0x0600 
#endif
#include <windows.h>

namespace sys {

	typedef LONG atomic32_t;

	inline atomic32_t atomic_get32 (atomic32_t volatile const * val)
	{
#if defined __MINGW32__
#else
		MemoryBarrier();
#endif
		return *val;
	}

	inline PVOID atomic_wrtptr (PVOID volatile * mem, PVOID val)
	{
		return InterlockedExchangePointer(mem, val);
	}


	inline atomic32_t atomic_cas32 (atomic32_t volatile * mem, atomic32_t oldv, atomic32_t newv)
	{
		return InterlockedCompareExchange(mem, newv, oldv);
	}

	inline atomic32_t atomic_faa32 (atomic32_t volatile * mem, atomic32_t val)
	{
		return InterlockedExchangeAdd(mem, val);
	}

	inline void lwsync () { }
}

