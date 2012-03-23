/**
 * Copyright (C) 2011 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Thanks to WarHorse Studios http://warhorsestudios.cz/ who funded initial ver
 **/
#pragma once

#if defined PROFILE_ENABLED

#	if defined (__GNUC__) && defined(__unix__)
#		define PROFILE_API __attribute__ ((__visibility__("default")))
#	elif defined (WIN32)
#		if defined PROFILE_STATIC
#			define PROFILE_API 
#		elif defined PROFILE_DLL
#			define PROFILE_API __declspec(dllexport)
#		else
#			define PROFILE_API __declspec(dllimport)
#		endif
#	endif

#	if  _MSC_VER == 1500
#		include <vadefs.h>
#	else
#		include <stdarg.h>	// for va_args
#	endif


/**	@macro		PROFILE_APPNAME
 *	@brief		sets application name that will be sent to server to identify
 **/
#	define PROFILE_APPNAME(name) profile::SetAppName(name)

/**	@macro		PROFILE_CONNECT
 *	@brief		connects to server and sends application name to server
 **/
#	define PROFILE_CONNECT() profile::Connect()

/**	@macro		PROFILE_DISCONNECT
 *	@brief		disconnects from server
 **/
#	define PROFILE_DISCONNECT() profile::Disconnect()

/**	@macro		PROFILE_SETLEVEL
 *	@brief		switch level to another value
 **/
#	define PROFILE_SETLEVEL(n) profile::SetRuntimeLevel(n)

/**	@macro		PROFILE_MSG
 *	@brief		logging of the form PROFILE_MSG(lvl, ctx, fmt, ...)
 **/
#	define PROFILE_BGN	profile::WriteBgn
#	define PROFILE_END   profile::WriteEnd

#	define PROFILE_FRAME_BGN   profile::WriteFrameBgn
#	define PROFILE_FRAME_END   profile::WriteFrameEnd


/**	@macro		PROFILE_MSG_VA
 *	@brief		profiling of the form PROFILE_MSG_VA(lvl, ctx, fmt, va_list)
 **/
#	define PROFILE_MSG_VA(fmt, vaargs)	\
		profile::WriteVA(fmt, vaargs);

/**	@macro		PROFILE_ENTRY
 *	@brief		profiles scope
 **/
#	define PROFILE_SCOPE(fmt, ... )	\
		profile::ScopedProfile entry_guard___(fmt, __VA_ARGS__)

	namespace profile {

		PROFILE_API void SetAppName (char const *);
		PROFILE_API char const * GetAppName ();

		PROFILE_API void Connect ();
		PROFILE_API void Disconnect ();

		/**@fn		BgnVA
		 * @brief	write begin to profiler in form (fmt, va_list)
		 **/
		PROFILE_API void WriteBgnVA (char const * fmt, va_list);

		/**@fn		Bgn
		 * @brief	write begin to profiler in form (fmt, ...)
		 **/
#if defined __GCC__ || defined __MINGW32__ || defined __linux__
		PROFILE_API void WriteBgn (char const * fmt, ...) __attribute__ ((format(printf, 1, 2) ));
#elif defined _MSC_VER
		PROFILE_API void WriteBgn (char const * fmt, ...);
#endif
		PROFILE_API void WriteBgn ();
		PROFILE_API void WriteEnd ();
		PROFILE_API void WriteEnd (char const * fmt, ...);

#if defined __GCC__ || defined __MINGW32__ || defined __linux__
		PROFILE_API void WriteFrameBgn (char const * fmt, ...) __attribute__ ((format(printf, 1, 2) ));
#elif defined _MSC_VER
		PROFILE_API void WriteFrameBgn (char const * fmt, ...);
#endif
		PROFILE_API void WriteFrameBgn ();
		PROFILE_API void WriteFrameEnd ();
		PROFILE_API void WriteFrameEnd (char const * fmt, ...);


		/**@class	ScopedProfile
		 * @brief	RAII class for profiling begin on construction and profiling end on destruction
		 **/
		struct PROFILE_API ScopedProfile
		{
			ScopedProfile (char const * fmt, ...);
			~ScopedProfile ();
		};
	}

#else // no profiling at all
#	define PROFILE_APPNAME(name)                      ((void)0)
#	define PROFILE_SETLEVEL(n)                        ((void)0)
#	define PROFILE_CONNECT()                          ((void)0)
#	define PROFILE_DISCONNECT()                       ((void)0)
#	define PROFILE_BGN(fmt, ... )                     ((void)0)
#	define PROFILE_BGN_VA(fmt, va)                    ((void)0)
#	define PROFILE_ENTRY(fmt, ...)                    ((void)0)
#endif // PROFILE_ENABLED
