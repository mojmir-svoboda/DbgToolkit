#include "wh_trace.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

DWORD WINAPI do_something ( LPVOID )
{
	TRACE_ENTRY(trace::e_Info, trace::CTX_Default);
	while (!g_Quit)
	{
		static size_t i = 0;
		++i;
		TRACE_MSG(trace::e_Info, trace::CTX_Default,  "Thread tick i=%u", i);
		something_useful();
		Sleep(100);
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

//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
int main ()
{
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
		Sleep(2000);
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
