#include <qlogging.h>
#include <stdarg.h>

#if defined REDIR_ASN_DEBUG
void ASN_DEBUG_f (const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char tmp[1024];
	if (vsnprintf_s(tmp, 1024, fmt, va) > 0)
		QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug("%s",tmp);
	va_end(va);
}
#endif
