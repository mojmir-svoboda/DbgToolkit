#pragma once

struct Bar
{
	Bar ()
	{
		TRACE_SCOPE(LL_INFO, CTX_Main);
	}

	~Bar ()
	{
		TRACE_SCOPE(LL_INFO, CTX_Main);
	}
};


