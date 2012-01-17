#define WIN32_LEAN_AND_MEAN
#if defined __MINGW32__
#	undef _WIN32_WINNT
#	define _WIN32_WINNT 0x0600 
#	include <windows.h>
#endif
#include <cstdio>
#include "../../tlv_parser/tlv_parser.h"

#define CACHE_LINE  32
#define CACHE_ALIGN __declspec(align(CACHE_LINE))

namespace trace {
	namespace sys {

		ULONGLONG g_TickStart = 0;
		inline ULONGLONG GetTickStart () { return g_TickStart; }
#if defined __MINGW32__
		void SetTickStart () { g_TickStart = ::GetTickCount(); }
		inline unsigned long long GetTime () { return ::GetTickCount() - GetTickStart(); }
#else
		void SetTickStart () { g_TickStart = ::GetTickCount64(); }
		inline unsigned long long GetTime () { return ::GetTickCount64() - GetTickStart(); }
#endif

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

		inline LONG atomic_get (LONG volatile * val)
		{
#if defined __MINGW32__
#else
			MemoryBarrier();
#endif
			return *val;
		}

		inline LONG atomic_cas32 (LONG volatile * mem, LONG with, LONG cmp)
		{
			return InterlockedCompareExchange(mem, with, cmp);
		}

		/**@brief	yields core to other thread **/
		inline void thread_yield () { Sleep(1); }

		/**@class	Message
		 * @brief	storage for message to be logged
		 *
		 * Message features 4-phase lifetime:
		 *	a) e_clean    --> e_writing
		 *		some thread is writing now in the buffer
		 *	b) e_writing  --> e_dirty
		 *		data are written and ready to be flushed
		 *	c) e_dirty    --> e_flushing
		 *	    consumer thread is flushing into storage or network
		 *	d) e_flushing --> e_clean
		 *		data are flushed and buffer can be reused
		 */
		CACHE_ALIGN struct Message
		{
			LONG mutable volatile m_lock;
			unsigned m_length;
			enum { e_data_sz = 1024 - sizeof(LONG) - sizeof(unsigned) };
			char m_data[e_data_sz];

			enum {
				e_clean    = 0,
				e_writing  = 1,
				e_dirty    = 2,
				e_flushing = 3,
			};

			void WriteUnlockAndDirty ()
			{
				atomic_cas32(&m_lock, e_dirty, e_writing);
			}

			void WriteLock ()
			{
				for (;;)
				{
					LONG prev_lock = atomic_cas32(&m_lock, e_writing, e_clean);
					if (m_lock == e_writing && prev_lock == e_clean)
						break;
					else
						thread_yield();
				}
			}

			void ReadLock ()
			{
				for (;;)
				{
					LONG prev_lock = atomic_cas32(&m_lock, e_flushing, e_dirty);
					if (m_lock == e_flushing && prev_lock == e_dirty)
						break;
					else
						thread_yield();
				}
			}

			void ReadUnlockAndClean ()
			{
				atomic_cas32(&m_lock, e_clean, e_flushing);
			}
		};

		/**@brief	simple pool of messages to be logged **/
		CACHE_ALIGN struct MessagePool
		{
			enum { e_size = 512 };
			Message m_msgs[e_size];

			MessagePool () { memset(this, 0, sizeof(*this)); }
			Message & operator[] (size_t i) { return m_msgs[i]; }
			Message const & operator[] (size_t i) const { return m_msgs[i]; }
		};

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
}

