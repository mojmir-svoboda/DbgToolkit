#pragma once

namespace trace {

	enum E_TraceLevel
	{
		e_Fatal = 0,
		e_Error,
		e_Warning,
		e_Info,
		e_Detail,
		e_Debug,
		e_Brutus,		// too much detail (per frame useless info etc)

		e_max_trace_level
	};

	typedef E_TraceLevel level_t;
}


