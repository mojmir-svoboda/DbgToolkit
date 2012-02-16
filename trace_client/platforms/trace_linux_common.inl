#pragma once
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE 1
#endif
#include <ctype.h>
#include <errno.h>
#include <cstdio>	// for vsnprintf etc
#include <cstdlib>	// for atoi
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define CACHE_LINE  
#define CACHE_ALIGN 
//#define CACHE_LINE  32
//#define CACHE_ALIGN __declspec(align(CACHE_LINE))

	namespace trace {
	namespace sys {

		struct timeval g_TickStart;
		inline timeval GetTickStart () { return g_TickStart; }

		void SetTickStart () { gettimeofday(&g_TickStart, NULL); }
		inline unsigned long long GetTime ()
		{
			struct timeval end;
			gettimeofday(&end, NULL);

			long const seconds	= end.tv_sec  - g_TickStart.tv_sec;
			long const useconds = end.tv_usec - g_TickStart.tv_usec;
			long const mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
			return mtime;
		}

		struct Timer {
			unsigned long long m_expire_at;

			Timer () : m_expire_at(0) { }
			void set_delay_ms (unsigned delay_ms) { m_expire_at = GetTime() + delay_ms; }
			void reset () { m_expire_at = 0; }
			bool enabled () const { return m_expire_at != 0; }
			bool expired () const { return GetTime() > m_expire_at; }
		};

		unsigned get_pid () { return getpid(); }
		unsigned get_tid () { return pthread_self(); } /// "hey piggy," i know
		void create_log_filename (char * filename, size_t buff_sz)
		{
			char const * app_name = GetAppName() ? GetAppName() : "unknown";
			snprintf(filename, buff_sz, "%s_%u.tlv_trace", app_name, get_pid());
		}

		inline tlv::len_t trc_vsnprintf (char * buff, size_t ln, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			int const n = vsnprintf(buff, ln, fmt, args);
			va_end(args);
			return static_cast<tlv::len_t>(n < 0 ? ln : n);
		}

		inline tlv::len_t va_trc_vsnprintf (char * buff, size_t ln, char const * fmt, va_list args)
		{
			int const n = vsnprintf(buff, ln, fmt, args);
			return static_cast<tlv::len_t>(n < 0 ? ln : n);
		}

		typedef uint32_t atomic32_t;
		inline atomic32_t atomic_get (atomic32_t volatile * val) { return *val; }
		inline atomic32_t atomic_cas32 (atomic32_t volatile * mem, atomic32_t with, atomic32_t cmp)
		{
			atomic32_t prev = cmp;
			// This version by Mans Rullgard of Pathscale
			__asm__ __volatile__ ( "lock\n\t"
								  "cmpxchg %2,%0"
								: "+m"(*mem), "+a"(prev)
								: "r"(with)
								: "cc");

			return prev;
		}
		inline atomic32_t atomic_add32 (atomic32_t volatile * mem, atomic32_t val)
		{
		   int r;
		   asm volatile
		   (
			  "lock\n\t"
			  "xadd %1, %0":
			  "+m"( *mem ), "=r"( r ): // outputs (%0, %1)
			  "1"( val ): // inputs (%2 == %1)
			  "memory", "cc" // clobbers
		   );
		   return r;
		}
		inline atomic32_t atomic_inc32 (atomic32_t volatile * mem) {  return atomic_add32(mem, 1);  }


		/**@brief	yields core to other thread **/
		inline void thread_yield () { pthread_yield(); }

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
			atomic32_t mutable volatile m_lock;
			unsigned m_length;
			enum { e_data_sz = 1024 - sizeof(atomic32_t) - sizeof(unsigned) };
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
					atomic32_t prev_lock = atomic_cas32(&m_lock, e_writing, e_clean);
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
					atomic32_t prev_lock = atomic_cas32(&m_lock, e_flushing, e_dirty);
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

		struct thread_info {    /* Used as argument to thread_start() */
			pthread_t thread_id;        /* ID returned by pthread_create() */
			int       thread_num;       /* Application-defined thread # */
			char     *argv_string;      /* From command-line argument */
		};

		/**@brief	simple encapsulation of CreateThread **/
#		define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#		define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)
		struct Thread
		{
			pthread_attr_t m_attr;
			thread_info m_tinfo;

			Thread () { memset(this, 0, sizeof(*this)); }
			~Thread () { Close(); }

			void Resume () { }
			void Create (void * (* fn) (void *), void * )
			{
				/* Initialize thread creation attributes */
				int s = pthread_attr_init(&m_attr);
				if (s != 0)
					handle_error_en(s, "pthread_attr_init");
				/*if (stack_size > 0)
				{
					s = pthread_attr_setstacksize(&m_attr, stack_size);
					if (s != 0)
						handle_error_en(s, "pthread_attr_setstacksize");
				}*/

				m_tinfo.thread_num = 0;
				//m_tinfo.argv_string = argv[optind + t];

				/* The pthread_create() call stores the thread ID into corresponding element of m_tinfo */
				s = pthread_create(&m_tinfo.thread_id, &m_attr, fn, &m_tinfo);
				if (s != 0)
					handle_error_en(s, "pthread_create");
			}
			void WaitForTerminate ()
			{
				/* Destroy the thread attributes object, since it is no longer needed */
				int s = pthread_attr_destroy(&m_attr);
				if (s != 0)
					handle_error_en(s, "pthread_attr_destroy");

				void * res = 0;
				s = pthread_join(m_tinfo.thread_id, &res);
				if (s != 0)
				   handle_error_en(s, "pthread_join");

				printf("Joined with thread %d; returned value was %s\n", m_tinfo.thread_num, (char *) res);
				free(res);      /* Free memory allocated by thread */
			}
			void Close () { }
		};
#		undef handle_error_en
#		undef handle_error



/*
		struct Thread
		{
			Thread (int prio) : m_handle(NULL), m_tid(), m_prio(prio) { }

			bool Create ( void * (* fn) (void *), void * arg)
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
		};*/

	}
}

