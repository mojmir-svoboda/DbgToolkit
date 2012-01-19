#pragma once
#ifdef _MSC_VER
#	include <cstdint>
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

