#include <stdarg.h>
#include <cstdio>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined REDIR_ASN_DEBUG
void ASN_DEBUG_f (const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char tmp[1024];
	vsnprintf_s(tmp, 1024, fmt, va);
	OutputDebugStringA(tmp);
	OutputDebugStringA("\n");
	va_end(va);
}
#endif
