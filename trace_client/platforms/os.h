#pragma once

#if defined WIN32 || defined WIN64 || defined _XBOX

#	if defined WIN32 || defined WIN64
#		define WIN32_LEAN_AND_MEAN
#		include <windows.h>
#	elif defined _XBOX
#		include <xtl.h>
#		include <winbase.h>
#	endif
	namespace sys
	{
#	if defined WIN32 || defined WIN64
		inline unsigned get_pid () { return GetCurrentProcessId(); }
#	elif defined _XBOX
		inline unsigned get_pid () { return 0; }
#	endif
		inline unsigned get_tid () { return GetCurrentThreadId(); }

		/**@brief	yields core to other thread **/
		inline void thread_yield () { Sleep(1); }

		inline tlv::len_t trc_vsnprintf (char * buff, size_t ln, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
#if defined __MINGW32__
			int const n = vsnprintf(buff, ln, fmt, args);
#else
			int const n = vsnprintf_s(buff, ln, _TRUNCATE, fmt, args);
#endif
			va_end(args);
			return static_cast<tlv::len_t>(n < 0 ? ln : n);
		}

		inline tlv::len_t va_trc_vsnprintf (char * buff, size_t ln, char const * fmt, va_list args)
		{
#if defined __MINGW32__
			int const n = vsnprintf(buff, ln, fmt, args);
#else
			int const n = vsnprintf_s(buff, ln, _TRUNCATE, fmt, args);
#endif
			return static_cast<tlv::len_t>(n < 0 ? ln : n);
		}

		/**@brief	simple encapsulation of CreateThread **/
		struct Thread
		{
			Thread (int prio) : m_handle(NULL), m_tid(), m_prio(prio) { }

			bool Create ( DWORD (WINAPI * fn) (void *), void * arg)
			{
				m_handle = CreateThread (
					0, // Security attributes
					0, // Stack size
					fn, arg,
					CREATE_SUSPENDED,
					&m_tid);
				if (m_handle)
					SetThreadPriority(m_handle, m_prio);

				return (m_handle != NULL);
			}

			~Thread () { }
			void Close () { if (m_handle) CloseHandle (m_handle); m_handle = 0; } 
			void Resume () { if (m_handle) ResumeThread (m_handle); }
			void WaitForTerminate ()
			{
				if (m_handle)
					WaitForSingleObject(m_handle, 2000);
			}
		private:
			HANDLE m_handle;
			DWORD  m_tid;
			int    m_prio;
		};
	}
#else

#	include <sys/time.h>
#	include <stdint.h>
#	include <stdbool.h>
#	include <stddef.h>

	namespace sys {
	}

#endif


