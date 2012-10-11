#pragma once
#if _MSC_VER == 1600
#	include <cstdint>
#elif _MSC_VER == 1500
	typedef unsigned long long uint64_t;
#else
#	include <inttypes.h>
#endif

namespace trace {

	typedef uint64_t context_t;

	// @NOTE: all the context types have moved into TraceContexts.inc

#define DRY(a,b) static context_t const a = b;
#	include "TraceContexts.inc"
#undef DRY

	struct CtxDictPair {
		char const * first;
		context_t second;
	};

	inline size_t getContextDictionnary (CtxDictPair const * & out)
	{
		static CtxDictPair s_dict[] = {
#define DRY(a,b) { #a , b },
#	include "TraceContexts.inc"
#undef DRY
		};
		out = s_dict;
		return sizeof(s_dict) / sizeof(*s_dict);
	}
}


