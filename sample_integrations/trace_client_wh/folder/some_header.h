#pragma once

namespace folder {
	inline void some_fn (int i)
	{

		switch (i % 4)
		{
			case 0: TRACE_MSG(trace::e_Info, trace::CTX_Default,  "yawn i=%u from some_fn, case 0", i); break;
			case 1: TRACE_MSG(trace::e_Info, trace::CTX_Default,  "yawn i=%u from some_fn, case 1", i); break;
			case 2: TRACE_MSG(trace::e_Info, trace::CTX_Default,  "yawn i=%u from some_fn, case 2", i); break;
			case 3: TRACE_MSG(trace::e_Info, trace::CTX_Default,  "yawn i=%u from some_fn, case 3", i); break;
		}
	}
}
