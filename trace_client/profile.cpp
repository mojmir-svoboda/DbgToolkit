#include "profile.h"
#include <stdarg.h>

#if defined PROFILE_ENABLED

	namespace profile {

		char const * g_AppName = "profile_client";

		// setup
		void SetAppName (char const * name) { g_AppName = name; }
		char const * GetAppName () { return g_AppName; }

		void WriteBgnVA (char const * fmt, va_list args);
		void WriteBgn (char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteBgnVA(fmt, args);
			va_end(args);
		}
		void WriteBgn_Impl ();
		void WriteBgn () { WriteBgn_Impl(); }

		void WriteEndVA (char const * fmt, va_list args);
		void WriteEnd (char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteEndVA(fmt, args);
			va_end(args);
		}
		void WriteEnd_Impl ();
		void WriteEnd () { WriteEnd_Impl(); }

		void WriteFrameBgnVA (char const * fmt, va_list args);
		void WriteFrameBgn (char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteFrameBgnVA(fmt, args);
			va_end(args);
		}
		void WriteFrameBgn_Impl();
		void WriteFrameBgn () { WriteFrameBgn_Impl(); }

		void WriteFrameEndVA (char const * fmt, va_list args);
		void WriteFrameEnd (char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteFrameEndVA(fmt, args);
			va_end(args);
		}
		void WriteFrameEnd_Impl ();
		void WriteFrameEnd () { WriteFrameEnd_Impl(); }

		ScopedProfile::ScopedProfile (char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteBgnVA(fmt, args);
			va_end(args);
		}
		
		ScopedProfile::~ScopedProfile ()
		{
			WriteEnd();
		}
	}
#	include "platforms/profile_select_platform.inl"

#else // profiling is NOT enabled
	; // the second perfect line
#endif
