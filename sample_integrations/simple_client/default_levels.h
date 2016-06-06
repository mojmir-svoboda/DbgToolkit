#pragma once

namespace trace {
	typedef unsigned int level_t;
}

// @NOTE: all the level types are in .inc file
#define DRY(a,b) static trace::level_t const a = b;
#	include "default_levels.inc"
#undef DRY

namespace trace {

	struct LvlDictPair {
		char const * first;
		level_t second;
	};

	inline size_t getLevelDictionnary (LvlDictPair const * & out)
	{
		static LvlDictPair s_dict[] = {
#define DRY(a,b) { #a , b },
#	include "default_levels.inc"
#undef DRY
		};
		out = s_dict;
		return sizeof(s_dict) / sizeof(*s_dict);
	}
}
