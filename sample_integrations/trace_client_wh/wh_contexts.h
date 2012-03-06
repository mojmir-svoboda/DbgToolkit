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

	static context_t const CTX_Default = (1 << 0);
	static context_t const CTX_Render  = (1 << 1);
	static context_t const CTX_Other   = (1 << 2);
	// ...
}

