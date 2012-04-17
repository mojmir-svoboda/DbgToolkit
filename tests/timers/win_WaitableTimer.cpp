#include "config.h"
#include <trace_client/profile.h>
#include "time_query.h"
#include <windows.h>
#include <stdio.h>

namespace sys {
	__int64 g_Start, g_Freq;
}


#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <tchar.h>
#include <exception>
#include <string>
#include <queue>
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define INFINITE        0xFFFFFFFF  // Infinite timeout

#ifndef NTAPI
typedef  WINAPI NTAPI;
#endif
typedef  LONG NTSTATUS;
typedef  LONG KPRIORITY;

typedef struct _PROCESS_BASIC_INFORMATION {
	PVOID Reserved1;
	void* PebBaseAddress;
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	ULONG_PTR ParentProcessId;
} PROCESS_BASIC_INFORMATION;

// Taken from: http://msdn.microsoft.com/en-us/library/
// aa380518(VS.85).aspx
typedef struct _LSA_UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, 
UNICODE_STRING, *PUNICODE_STRING;


typedef struct _OBJECT_TYPE_INFORMATION {
	UNICODE_STRING TypeName;
	ULONG TotalNumberOfHandles;
	ULONG TotalNumberOfObjects;
}OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

// Returned by the ObjectAllTypeInformation class
// passed to NtQueryObject
typedef struct _OBJECT_ALL_INFORMATION {
	ULONG NumberOfObjects;
	OBJECT_TYPE_INFORMATION ObjectTypeInformation[1];
}OBJECT_ALL_INFORMATION, *POBJECT_ALL_INFORMATION;

// CheckCloseHandle will call CloseHandle on an invalid
// DWORD aligned value and if a debugger is running an exception
// will occur and the function will return true otherwise it'll
// return false
inline bool CheckDbgPresentCloseHandle()
{
	HANDLE Handle = (HANDLE)0x8000;
	try{
		CloseHandle(Handle);
	}catch(...){
                MessageBox(0,"DBG?","MSG",0);
                 TerminateProcess       (GetCurrentProcess   (),0);
	}
                MessageBox(0,"OK!","MSG",0);
	return 0;
}

NTSTATUS (NTAPI *NtDelayExecution)(ULONG bAlertable,PLARGE_INTEGER pDuration);

BOOL NtDelayExecutionEx(ULONG bAlertable,DWORD dwMilliseconds)
{
     LARGE_INTEGER Interval;
     HMODULE hNtDLL = GetModuleHandle("ntdll.dll");
     *(FARPROC*)&NtDelayExecution = GetProcAddress(hNtDLL,"NtDelayExecution");
     Interval.QuadPart = (unsigned __int64)dwMilliseconds * 10 * 100;
     return NtDelayExecution (bAlertable, &Interval);
}


int QuadInit(int n)
{
	if(IsDebuggerPresent() != 0)
	{
		MessageBox(0,"shit","",0);
		TerminateProcess(GetCurrentProcess   (),0);
	}

	NtDelayExecutionEx(FALSE, n);
	return 0;   
}

int main()
{
    HANDLE hTimer = NULL;
    LARGE_INTEGER liDueTime;

	using namespace wh;
	
	setTimeStart();

	WH_PROFILER_APPNAME("waitable timer");
    WH_PROFILER_CONNECT();
	WH_PROFILE_BEGIN("%s","main");

	printf("{ %s\n", __FUNCTION__);
#if defined WIN32
   setvbuf(stdout, 0, _IONBF, 0);
#endif

	//Sleep(500);

	for (size_t i = 0; i < 16; ++i)
	{
		WH_PROFILE_FRAME_BGN("test");
		/*{
			WH_PROFILE_SCOPE("mk timer");
			liDueTime.QuadPart = -10000LL;

			// Create an unnamed waitable timer.
			hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
			if (NULL == hTimer)
			{
				printf("CreateWaitableTimer failed (%d)\n", GetLastError());
				return 1;
			}
		}

		WH_PROFILE_BEGIN("mk timer");
		// Set a timer to wait for 10 seconds.
		if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
		{
			printf("SetWaitableTimer failed (%d)\n", GetLastError());
			return 2;
		}
		WH_PROFILE_END();

		// Wait for the timer.
		WH_PROFILE_BEGIN("w8");
		if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
			printf("WaitForSingleObject failed (%d)\n", GetLastError());
		WH_PROFILE_END();*/


		WH_PROFILE_BEGIN("nt w8");
		QuadInit(i);
		WH_PROFILE_END();

		WH_PROFILE_FRAME_END();

	}
	WH_PROFILE_END();

	Sleep(1000);
    WH_PROFILER_DISCONNECT();
	//std::cout << result[0] << std::endl;
    return 0;
}

