#define _WIN32_WINNT 0x0600 
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>

ULONGLONG g_TickStart = 0;
inline ULONGLONG GetTickStart () { return g_TickStart; }
void SetTickStart () { g_TickStart = GetTickCount64(); }
inline unsigned long long GetTime () { return GetTickCount64() - GetTickStart(); }

struct Timer {
	unsigned long long m_expire_at;
	Timer () : m_expire_at(0) { }
	void set_delay_ms (unsigned delay_ms) { m_expire_at = GetTime() + delay_ms; }
	
	bool enabled () const { return m_expire_at != 0; }
	bool expired () const { return GetTime() > m_expire_at; }
};

int main ()
{
	SetTickStart();

	Timer t;
	t.set_delay_ms(1000);
	while (1)
	{
		printf("tick! expired=%u\n", t.expired());
		if (t.expired())
			t.set_delay_ms(2000);

		Sleep(250);
		
	}
}
