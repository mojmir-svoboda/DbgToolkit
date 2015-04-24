#pragma once

enum E_TraceLevel
{
	e_Verbose,
	e_Debug,
	e_Info,
	e_Warning,
	e_Error,
	e_Fatal,

	e_max_trace_level	/// last item of enum
};

namespace trace {
	typedef E_TraceLevel level_t;
}

struct LvlDictPair {
  char const * first;
  trace::level_t second;
};

inline size_t getLevelDictionnary (LvlDictPair const * & out)
{
  static LvlDictPair s_dict[] =
  {
    { "verbose", e_Verbose },
    { "debug",   e_Debug   },
    { "Normal",  e_Info  },
    { "WARN",    e_Warning },
    { "ERROR",   e_Error   },
    { "FATAL",   e_Fatal   },
  };

  out = s_dict;
  return sizeof(s_dict) / sizeof(*s_dict);
}

