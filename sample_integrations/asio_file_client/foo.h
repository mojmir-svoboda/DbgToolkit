#pragma once

inline void foo ()
{
	TRACE_SCOPE(LL_INFO, CTX_Main);
	TRACE_MSG(LL_INFO, CTX_Main,  "%s %s", "\'llo world froom foo().", "and from bar!");
}


