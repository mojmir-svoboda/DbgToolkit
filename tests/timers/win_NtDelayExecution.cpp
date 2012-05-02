#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <tchar.h>
#include <exception>
#include <string>
#include <queue>
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define INFINITE        0xFFFFFFFF  // Infinite timeout

#define QuadInit           main

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

BOOL NtDelayExecutionEx(ULONG bAlertable,DWORD dwMilliseconds){
     LARGE_INTEGER Interval;
     HMODULE hNtDLL = GetModuleHandle("ntdll.dll");
     *(FARPROC*)&NtDelayExecution = GetProcAddress(hNtDLL,"NtDelayExecution");
     Interval.QuadPart = -(unsigned __int64)dwMilliseconds * 10000 * 1000;
     NtDelayExecution (bAlertable, &Interval);
     }


int QuadInit(){
 if(IsDebuggerPresent() != 0){MessageBox(0,"shit","",0);
 TerminateProcess       (GetCurrentProcess   (),0);
}
NtDelayExecutionEx(FALSE, 1);
MessageBoxEx(0,"what the fuck? :D:D","",0, 0);
 return 0;   
}
