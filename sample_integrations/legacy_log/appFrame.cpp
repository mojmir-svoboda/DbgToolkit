/*#include "lib/wpch.hpp"*/
//#include <Es/essencepch.hpp>
#include "appFrame.hpp"

#include <stdio.h>
#include <string.h>
//#include <Es/Common/win.h>
//#include <Es/Strings/bString.hpp>
//#include <system/enf_timer.h>

#if COUNT_CLASS_INSTANCES

PUBLIC_API AllCountersLink *AllCountInstances[];
int AllCountInstancesCount;

/// limit for WalkAllInstanceCounts
static int LimitInstanceCount = 100;

/// this function can be used from Debugger to get a list of interesting (often used) objects
void WalkAllInstanceCounts()
{
  for (int i=0; i<AllCountInstancesCount; i++)
  {
    int count = AllCountInstances[i]->GetValue();
    if (count>=LimitInstanceCount)
    {
      LogF("(*AllCountInstances[%d]):%d",i,count);
    }
  }
}

#endif

#if _SUPER_RELEASE
	bool DisableLogs = true;
#else
	bool DisableLogs = false;
#endif


#if _ENABLE_REPORT
# if defined TRACE_ENABLED
void LogMessage(trace::level_t level, trace::context_t context, char const * file, int line, char const * fn,  char const *format,...)
{
  if (DisableLogs) return;
	va_list arglist;
	va_start(arglist, format);
	trace::WriteVA(level, context, file, line, fn, format, arglist);
	va_end(arglist);
}

void vaLogMessage(trace::level_t level, trace::context_t context, char const * file, int line, char const * fn, const char *format, va_list argptr)
{
  if (DisableLogs) return;
	trace::WriteVA(level, context, file, line, fn, format, argptr);
}
# else
void LogF(char const *format,...)
{
  if (DisableLogs) return;

	va_list arglist;
	va_start(arglist, format);
	CurrentAppFrameFunctions->LogF(format, arglist);
	va_end(arglist);
}

void vaLogF(const char *format, va_list argptr)
{
  if (DisableLogs) return;
	CurrentAppFrameFunctions->LogF(format, argptr);
}
void ErrF(char const *format,...)
{
	va_list arglist;
	va_start(arglist, format);
	CurrentAppFrameFunctions->ErrF(format, arglist);
	va_end(arglist);
}

void vaErrF(const char *format, va_list argptr)
{
	CurrentAppFrameFunctions->ErrF(format, argptr);
}
# endif
#else
void ErrF(char const *format,...)
{
  va_list arglist;
  va_start(arglist, format);
  CurrentAppFrameFunctions->ErrF(format, arglist);
  va_end(arglist);
}

void vaErrF(const char *format, va_list argptr)
{
  CurrentAppFrameFunctions->ErrF(format, argptr);
}
#endif


void LstF(char const *format,...)
{
  if (DisableLogs) return;

	va_list arglist;
	va_start(arglist, format);

	CurrentAppFrameFunctions->LstF(format, arglist);

	va_end(arglist);
}

void vaLstF(const char *format, va_list argptr)
{
  if (DisableLogs) return;
	CurrentAppFrameFunctions->LstF(format, argptr);
}

void LogDebugger(char const *format,...)
{
  if (DisableLogs) return;

	va_list arglist;
	va_start(arglist, format);

	CurrentAppFrameFunctions->LogDebugger(format, arglist);

	va_end(arglist);
}

void ErrorMessage(ErrorMessageLevel level, char const *format,...)
{
	va_list arglist;
	va_start(arglist, format);

	CurrentAppFrameFunctions->ErrorMessage(level, format, arglist);

	va_end(arglist);
}

void ErrorMessage(char const *format,...)
{
	va_list arglist;
	va_start(arglist, format);

	CurrentAppFrameFunctions->ErrorMessage(format, arglist);

	va_end(arglist);
}

void WarningMessage(char const *format,...)
{
  if (DisableLogs) return;
	va_list arglist;
	va_start(arglist, format);

	CurrentAppFrameFunctions->WarningMessage(format, arglist);

	va_end(arglist);
}

void ResetMainThread()
{
	CurrentAppFrameFunctions->ResetMainThread();
}

bool CheckMainThread()
{
  return CurrentAppFrameFunctions->CheckMainThread();
};
bool CheckMainThread(int id)
{
  return CurrentAppFrameFunctions->CheckMainThread(id);
};
bool CheckSameThread(int &id)
{
  return CurrentAppFrameFunctions->CheckSameThread(id);
};

void GlobalDiagMessage(int &handle, int id, int timeMs, const char *msg, ...)
{
	va_list arglist;
	va_start(arglist, msg);
	
/*  BString<512> buf;*/
	char buf[512];
  vsprintf(buf,msg,arglist);
  
	CurrentAppFrameFunctions->DiagMessage(handle,id,timeMs, buf);

	va_end(arglist);
}

// void GlobalDiagMessagePos(int x, int y, const PackedColor& color, int maxChars, const char *msg, ...)
// {
// 	va_list arglist;
// 	va_start(arglist, msg);
// 
// 	BString<512> buf;
// 	vsprintf(buf,msg,arglist);
// 
// 	CurrentAppFrameFunctions->DiagMessage(x, y, color, maxChars, buf);
// 
// 	va_end(arglist);
// }
// 
// void GlobalShowMessage(int timeMs, const char *msg, ...)
// {
// 	va_list arglist;
// 	va_start(arglist, msg);
// 
//   BString<512> buf;
//   vsprintf(buf,msg,arglist);
// 
// 	CurrentAppFrameFunctions->ShowMessage(timeMs, buf);
// 
// 	va_end(arglist);
// }


// void GlobalShowMessage(int timeMS, const char *text, InitVal<int,-1> *handles, int n)
// {
//   const int line = 80;
//   int i = 0;
// 
//   // split message to multiple lines when contain \n
//   const char *start = text;
//   while (true)
//   {
//     const char *ptr = strchr(start, '\n');
//     if (ptr && ptr - start <= line)
//     {
//       // next line
//       GlobalDiagMessage(handles[i], 0, timeMS, "%s", cc_cast(RString(start, ptr - start)));
//       if (i + 1 < n) i++;
//       start = ptr + 1;
//     }
//     else
//     {
//       if (strlen(start) <= line)
//       {
//         GlobalDiagMessage(handles[i], 0, timeMS, "%s", start);
//         break;
//       }
//       // line too long
//       const char *space = start + line - 1;
//       while (space > start && !isspace(*space)) space--;
//       RString str;
//       if (space == start)
//       {
//         // no space on this line
//         str = RString(start, line);
//         start += line;
//       }
//       else
//       {
//         // word wrap
//         str = RString(start, space - start);
//         start = space + 1;
//       }
//       GlobalDiagMessage(handles[i], 0, timeMS, "%s", cc_cast(str));
//       if (i + 1 < n) i++;
//     }
//   }
// }

// Default implementation

#if _ENABLE_REPORT

# if defined TRACE_ENABLED
void AppFrameFunctions::LogMessage(trace::level_t level, trace::context_t context, char const * file, int line, char const * fn, const char *format, va_list argptr)
{
  if (DisableLogs) return;
  trace::WriteVA(level, context, file, line, fn, format, argptr);
}
#else
void AppFrameFunctions::LogF(const char *format, va_list argptr)
{
  if (DisableLogs) return;

	char buf[512];
	vsprintf(buf,format,argptr);
	strcat(buf,"\n");
#ifdef _WIN32
  #ifdef UNICODE
    WCHAR wBuf[512];
    MultiByteToWideChar(CP_ACP, 0, buf, -1, wBuf, lenof(wBuf));
#   if defined TRACE_ENABLED
      trace::WriteVA(LL_NORMAL, CTX_LEGACY, __FILE__, __LINE__, __FUNCTION__, format, argptr);
#   else
    OutputDebugString(wBuf);
#   endif
  #else
    OutputDebugString(buf);
  #endif // !UNICODE
#else
	fputs(buf,stderr);
#endif
}
void AppFrameFunctions::ErrF(const char *format, va_list argptr)
{
  LogF(format, argptr);
}


#endif

void AppFrameFunctions::LogDebugger(const char *format, va_list argptr)
{
  LogF(format, argptr);
}

#else

void AppFrameFunctions::ErrF(const char *format, va_list argptr)
{
  LogF(format, argptr);
}

void AppFrameFunctions::LogDebugger(const char *format, va_list argptr)
{
	#ifdef _WIN32
# if defined TRACE_ENABLED
  trace::WriteVA(LL_NORMAL, CTX_LEGACY, __FILE__, __LINE__, __FUNCTION__, format, argptr);
# else
	BString<512> buf;
	vsprintf(buf,format,argptr);
	strcat(buf,"\n");
	OutputDebugString(buf);
	#endif
	#endif
}

#endif

void AppFrameFunctions::LstF(const char *format, va_list argptr)
{
#if _ENABLE_REPORT
  LogF(format, argptr);
#endif
}

void AppFrameFunctions::LstFDebugOnly(const char *format, va_list argptr)
{
#if _ENABLE_REPORT
  LogF(format, argptr);
#endif
}

#ifdef _WIN32
#ifdef ENF_XBOXONE
static DWORD MainThreadId = 0;
static DWORD MainX1ThreadId = GetCurrentThreadId();;
#else
static DWORD MainThreadId = GetCurrentThreadId();
#endif

bool AppFrameFunctions::CheckMainThread(int threadId) const
{
#ifdef ENF_XBOXONE
  return threadId == MainThreadId || threadId == MainX1ThreadId;
#else
  return threadId == MainThreadId;
#endif
}

int AppFrameFunctions::GetMainThreadId() const
{
  return MainThreadId;
}

bool AppFrameFunctions::CheckMainThread() const
{
  // if MainThreadId not initialized yet, we are unable to verify
#ifdef ENF_XBOXONE
  return MainThreadId == 0 || GetCurrentThreadId() == MainThreadId || GetCurrentThreadId() == MainX1ThreadId;
#else
  return MainThreadId == 0 || GetCurrentThreadId() == MainThreadId;
#endif
}
bool AppFrameFunctions::CheckSameThread(int &id) const
{
  // if MainThreadId not initialized yet, we are unable to verify
  DWORD currId = GetCurrentThreadId();
  if (id==0)
  {
    // when called for the first time, initialize
    id = currId;
    return true;
  }
  else
  {
#ifdef ENF_XBOXONE
    if (CheckMainThread(currId) && CheckMainThread(id))
      return true;
#endif
    return currId==id;
  }
}
void AppFrameFunctions::ResetMainThread()
{
  MainThreadId = GetCurrentThreadId();
}

#else

int AppFrameFunctions::GetMainThreadId() const
{
  //unconst_cast(this)->Fail("Not implemented");
  return 0;
}

bool AppFrameFunctions::CheckMainThread(int threadId) const
{
  //unconst_cast(this)->Fail("Not implemented");
  return true;
}

bool AppFrameFunctions::CheckMainThread() const
{
  return true;
}
bool AppFrameFunctions::CheckSameThread(int &id) const
{
  return true;
}
void AppFrameFunctions::ResetMainThread()
{

}

#endif

void LogFForced(float delay, char const *text,...)
{
//   if (DisableLogs)
//   {
//     static Time lastReport(0);
// 
//     if (lastReport + delay < Glob.time) // when logs disabled report occasionally after delay passed
//     {
//       DisableLogs = false;
//       va_list arglist;
//       va_start(arglist, text);
//       LstF(text, arglist);
//       va_end(arglist);
//       DisableLogs = true;
//       lastReport = Glob.time;
//     }
//   }
//   else
//   {
//     va_list arglist;
//     va_start(arglist, text);
//     LstF(text, arglist);
//     va_end(arglist);
//   }
}

