#pragma once

void my_custom_vaarg_fn (char const * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TRACE_MSG_VA(LL_INFO, CTX_Main, fmt, args);
	va_end(args);
}


