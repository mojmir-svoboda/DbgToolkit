#pragma once

namespace trace {

	enum E_TraceLevel
	{
		e_Fatal = 0,
		e_Error,
		e_Warning,
		e_Info,
		e_Detail,
		e_Debug
	};

	typedef E_TraceLevel level_t;
}


