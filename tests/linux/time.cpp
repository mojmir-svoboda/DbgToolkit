#include <sys/time.h>
#include <cstdio>
#include <unistd.h>

namespace trace {
    namespace sys {

        struct timeval g_TickStart;
        inline timeval GetTickStart () { return g_TickStart; }

        void SetTickStart () { gettimeofday(&g_TickStart, NULL); }
        inline unsigned long long GetTime ()
		{
			struct timeval end;
			gettimeofday(&end, NULL);

			long const seconds  = end.tv_sec  - g_TickStart.tv_sec;
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
	}
}

int main()
{
	trace::sys::SetTickStart();
    struct timeval end;

	usleep(5000000);

    printf("Elapsed time: %lld milliseconds\n", trace::sys::GetTime());

    return 0;
}
