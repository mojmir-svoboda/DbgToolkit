#include "trace.h"
#include <stdarg.h>
#include "platforms/time_query.h"

#if defined TRACE_ENABLED

	namespace sys {
		hptimer_t g_Start = 0, g_Freq = 1000000;
	}

	namespace trace {

		level_t      g_RuntimeLevel       = static_cast<level_t>(e_max_trace_level);
		context_t    g_RuntimeContextMask = ~(0U);
		char const * g_AppName            = "trace_client";
		bool         g_RuntimeBuffering   = true;

		// setup
		void SetAppName (char const * name) { g_AppName = name; }
		char const * GetAppName () { return g_AppName; }

		void SetRuntimeLevel (level_t level) { g_RuntimeLevel = level; }
		level_t GetRuntimeLevel () { return g_RuntimeLevel; }

		void SetRuntimeBuffering (bool buffered) { g_RuntimeBuffering = buffered; }
		bool GetRuntimeBuffering () { return g_RuntimeBuffering; }

		void SetRuntimeContextMask (context_t mask) { g_RuntimeContextMask = mask; }
		context_t GetRuntimeContextMask () { return g_RuntimeContextMask; }

		// message logging
		void WriteLog (level_t level, context_t context, char const * file, int, char const *, char const *, va_list);
		void WriteVA (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteLog(level, context, file, line, fn, fmt, args);
		}
		void Write (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteVA(level, context, file, line, fn, fmt, args);
			va_end(args);
		}

		// scope logging
		inline void WriteScope (ScopedLog::E_Type type, level_t level, context_t context, char const * file, int line, char const * fn);
		
		ScopedLog::ScopedLog (level_t level, context_t context, char const * file, int line, char const * fn)
			: m_level(level), m_context(context), m_file(file), m_line(line), m_fn(fn)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteScope(e_Entry, level, context, file, line, fn);
		}
		
		ScopedLog::~ScopedLog ()
		{
			if (RuntimeFilterPredicate(m_level, m_context))
				WriteScope(e_Exit, m_level, m_context, m_file, m_line, m_fn);
		}
	}
#	include "platforms/select_platform.inl"

#else // tracing is NOT enabled
	; // the perfect line
#endif
