#include <stdarg.h>
#include <cstdio>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined REDIR_ASN_DEBUG
void ASN_DEBUG_f (const char *fmt, ...)
{
	char buff[4096];
	va_list argptr;
	va_start(argptr, fmt);
	int n = _vsnprintf_s(buff, 4096, fmt, argptr);
	va_end(argptr);
	if (n > 0)
	{
		buff[n] = '\n';
		buff[n + 1] = '\0';
		OutputDebugStringA(buff);
	}
}
#endif
