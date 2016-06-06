#pragma once

struct Bar
{
	Bar ()
	{
		TRACE_SCOPE(LL_NORMAL, CTX_Default);
	}

	~Bar ()
	{
		TRACE_SCOPE(LL_NORMAL, CTX_Default);
	}
};


