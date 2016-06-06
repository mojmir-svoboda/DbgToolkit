#pragma once

namespace trace {
	typedef unsigned int context_t;
}

	// @NOTE: all the context types are in .inc file
#define DRY(a,b) static trace::context_t const a = b;
#	include "default_contexts.inc"
#undef DRY

namespace trace {

	struct CtxDictPair {
		char const * first;
		context_t second;
	};

	inline size_t getContextDictionnary (CtxDictPair const * & out)
	{
		static CtxDictPair s_dict[] = {
#define DRY(a,b) { #a , b },
#	include "default_contexts.inc"
#undef DRY
		};
		out = s_dict;
		return sizeof(s_dict) / sizeof(*s_dict);
	}
}
