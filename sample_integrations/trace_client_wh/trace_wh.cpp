#include "wh_trace.h"

#if defined WIN32 || defined WIN64
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <cstdio>

namespace {
	template<size_t N>
	struct ThreadPool
	{
		enum { e_thread_count = N };
		DWORD	m_tids[e_thread_count];
		HANDLE	m_handles[e_thread_count];

		ThreadPool () { memset(this, 0, sizeof(*this)); }
		~ThreadPool () { Close(); }
		void Create (DWORD (WINAPI * fn) (void *), void * ) {
			TRACE_ENTRY(trace::e_Info, trace::CTX_Default);
			for (size_t i = 0; i < e_thread_count; i++ )
				m_handles[i] = CreateThread( NULL, 0, fn, 0, 0, &m_tids[i]); 
		}
		void WaitForTerminate () { WaitForMultipleObjects(e_thread_count, m_handles, TRUE, INFINITE); }
		void Close () {
			for (size_t i = 0; i < e_thread_count; i++)
				CloseHandle(m_handles[i]);
		}
	};
}

#elif defined __linux__
#	include <pthread.h>
#	include <cstring>
#	include <cstdio>
#	include <cstdlib>
#	include <unistd.h>
#	include <errno.h>
#	include <ctype.h>
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE 1
#	endif

#	define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#	define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

	struct thread_info {    /* Used as argument to thread_start() */
		pthread_t thread_id;        /* ID returned by pthread_create() */
		int       thread_num;       /* Application-defined thread # */
		char     *argv_string;      /* From command-line argument */
	};

	template<size_t N>
	struct ThreadPool
	{
		enum { e_thread_count = N };
		pthread_attr_t m_attr;
		thread_info m_tinfo[e_thread_count];

		ThreadPool () { memset(this, 0, sizeof(*this)); }
		~ThreadPool () { Close(); }
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

			/* Create one thread for each command-line argument */
			for (size_t t = 0; t < N; ++t)
			{
				m_tinfo[t].thread_num = t + 1;
				//m_tinfo[t].argv_string = argv[optind + t];

				pthread_attr_t m_attr;
				pthread_attr_init(&m_attr);
				cpu_set_t cpuset;

				CPU_ZERO(&cpuset);
				CPU_SET(t, &cpuset);
				//CPU_SET(atoi(m_tinfo[t].argv_string), &cpuset);
				pthread_attr_setaffinity_np(&m_attr, sizeof(cpuset), &cpuset);

				/* The pthread_create() call stores the thread ID into corresponding element of m_tinfo[] */
				s = pthread_create(&m_tinfo[t].thread_id, &m_attr, fn, &m_tinfo[t]);
				if (s != 0)
					handle_error_en(s, "pthread_create");
			}
		}

		void WaitForTerminate ()
		{
			/* Destroy the thread attributes object, since it is no longer needed */
			int s = pthread_attr_destroy(&m_attr);
			if (s != 0)
				handle_error_en(s, "pthread_attr_destroy");

			/* Now join with each thread, and display its returned value */
			for (size_t t = 0; t < N; t++)
			{
				void * res = 0;
				s = pthread_join(m_tinfo[t].thread_id, &res);
				if (s != 0)
				   handle_error_en(s, "pthread_join");

				printf("Joined with thread %d; returned value was %s\n", m_tinfo[t].thread_num, (char *) res);
				free(res);      /* Free memory allocated by thread */
			}

		}
		void Close ()
		{
		}
	};

#endif


void my_custom_vaarg_fn (char const * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TRACE_MSG_VA(trace::e_Info, trace::CTX_Default, fmt, args);
	va_end(args);
}

void something_useful_too ()
{
	TRACE_ENTRY(trace::e_Info, trace::CTX_Default);
	TRACE_MSG(trace::e_Info, trace::CTX_Default,  "%s", "Worker thread issues some another annoying message");
}

void something_useful ()
{
	TRACE_ENTRY(trace::e_Info, trace::CTX_Default);
	something_useful_too();
}

unsigned g_Quit = 0;

#if defined WIN32 || defined WIN64
DWORD WINAPI do_something ( LPVOID )
#elif defined __linux__
void * do_something ( void * )
#endif
{
	TRACE_ENTRY(trace::e_Info, trace::CTX_Default);
	while (!g_Quit)
	{
		static size_t i = 0;
		++i;
		TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Thread tick i=%u", i);
		something_useful();
#if defined WIN32 || defined WIN64
		Sleep(300);
#elif defined __linux__
		usleep(300 * 1000);
#endif
	}
	return 0;
}

void foo ()
{
	TRACE_ENTRY(trace::e_Info, trace::CTX_Default);
	TRACE_MSG(trace::e_Info, trace::CTX_Default,  "%s %s", "\'lloo woorld froom foo().", "and from bar!");
}

struct Bar
{
	Bar ()
	{
		TRACE_ENTRY(trace::e_Info, trace::CTX_Default);
	}

	~Bar ()
	{
		TRACE_ENTRY(trace::e_Info, trace::CTX_Default);
	}
};

#if defined WIN32 || defined WIN64
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
int main ()
#elif defined __linux__
int main ()
#endif
{
#if defined WIN32
	setvbuf(stdout, 0, _IONBF, 0);
#endif
	TRACE_APPNAME("WarHorse_App");
	TRACE_CONNECT();
	//TRACE_MSG(trace::e_Info, trace::CTX_Default,	"first message"); // not sure if this is a valid case!
	TRACE_MSG(trace::e_Info, trace::CTX_Default,	"this is %s", "first message");
	TRACE_MSG(trace::e_Info, trace::CTX_Default,  "args: %s and %s", "first arg", "second arg");
	TRACE_MSG(trace::e_Fatal, trace::CTX_Default,  "First fatal error, errno=%08x", 0xDEAFDAD);
	TRACE_MSG(trace::e_Error, trace::CTX_Default,  "First error, errno=%08x", 0xBADBEEF);
	TRACE_MSG(trace::e_Warning, trace::CTX_Default,  "First warning, errno=%x", 0xFEEDDEAD);
	TRACE_MSG(trace::e_Detail, trace::CTX_Default,	"%s%s", "This message should not appear", ".");
	TRACE_MSG(trace::e_Info, trace::CTX_Default,  "%s", "This message should appear.");
	TRACE_MSG(trace::e_Info, trace::CTX_Default,  "%s", "This message should partially appear too, but it's much longer in time and space so that it's very annoying and everyone will hate it as i do hate it now during typing as approaching to some 256 bytes boundary on which this message will be clipped and therefore it does not make any sense at all as all it does do is to show you in a rather graphomaniac light like Robert Smith or this Rowling bitch");
	my_custom_vaarg_fn("using va_arg macro %s and %s", "with some argument", "another one");

	foo();	
	Bar bar;

	ThreadPool<2> thr_pool;
	thr_pool.Create(do_something, 0);

	for (;;)
	{
#if defined WIN32 || defined WIN64
		Sleep(2000);
#elif defined __linux__
		usleep(2000 * 1000);
#endif

		TRACE_MSG(trace::e_Info, trace::CTX_Default,	"%s", "This message should periodicaly appear too.");
		
		//for(size_t i = 0; i < 4; ++i)
		static size_t i = 0;
		++i;
		TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Some another annoying message i=%u from main thread", i);

		//if (i == 4)
		//	break;
	}

	g_Quit = 1;
	thr_pool.WaitForTerminate();
	TRACE_DISCONNECT();
}
