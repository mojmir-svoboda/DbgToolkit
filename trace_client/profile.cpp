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

		void WriteFrameBgnVA (char const * fmt, va_list args);
		void WriteFrameBgn (char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteFrameBgnVA(fmt, args);
			va_end(args);
		}

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

		void link_foo () // @TODO: grr, just to force link - it's late and i'm lazy
		{
			WriteFrameEnd ();
		}

	}
#	include "platforms/profile_select_platform.inl"

#else // profiling is NOT enabled
	; // the second perfect line
#endif
