#pragma once

inline void foo ()
{
	TRACE_SCOPE(LL_NORMAL, CTX_Default);
	TRACE_MSG(LL_NORMAL, CTX_Default,  "%s %s", "\'llo world froom foo().", "and from bar!");
}


