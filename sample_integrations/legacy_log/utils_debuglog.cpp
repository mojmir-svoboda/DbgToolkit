//#include "lib/wpch.hpp"
//#include "utils_debuglog.h"
#ifdef _WIN32
//#	include <El/Debugging/imexhnd.h>
#endif
//#include <El/Debugging/debugTrap.hpp>
//#include "gameDirs.hpp"
#include "appFrameExt.hpp"
#if defined(ENF_PS4)
# include <cstdio>
#endif

extern bool DisableLogs; // disable logs (for autotest, enabled only logs from script)
#if defined(_WIN32)
//extern WINDOW_HANDLE hwndApp;
#endif
//extern RString InstanceName;
bool Silent = false; /// sometimes we want no error messages (automated builds)
//void DDTermError();

#if _ENABLE_REPORT
const char *LogFilename = "debug.log";
bool ResetLogFile = false;
//static LogFile GLogFile;
#endif

void OFPFrameFunctions::ErrorMessage(const char *format, va_list argptr)
{
  static int avoidRecursion=0;
  if( avoidRecursion++!=0 ) return;

//  GDebugger.NextAliveExpected(15*60*1000);

//  BString<256> buf;
	char buf[256];
  vsprintf( buf, format, argptr );

  // kill direct draw
#if defined _WIN32 && !defined _XBOX_ONE
  //GDebugExceptionTrap.SaveContext("ErrorMessage");
#endif
#if _ENABLE_REPORT
  // error which will terminate the game should break into the debugger
  ::ErrF("ErrorMessage: %s",(const char *)buf);
#else
#if defined _WIN32 && !defined(_XBOX_ONE)
  GDebugExceptionTrap.LogLine("ErrorMessage: %s",buf.cstr());
#else
  // !!! not yet !!!
#endif
#endif
#if defined(_WIN32)  && !defined(_XBOX_ONE)
//  if (hwndApp)
//		DestroyWindow((HWND)hwndApp);
#endif
//  DDTermError();
  // kill main application window
  //MessageBox(hwndApp,buf,AppName,MB_OK);
  //WinDestroy();
  // show message
#if defined _WIN32 && !defined(_XBOX_ONE)
  //if( HideCursor ) ShowCursor(TRUE);

//   // convert message from UTF-8 to Unicode
//   int len = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
//   AUTO_STATIC_ARRAY(wchar_t, wBuffer, 1024);
//   wBuffer.Resize(len);
//   MultiByteToWideChar(CP_UTF8, 0, buf, -1, wBuffer.Data(), len);
//   wBuffer[len - 1] = 0; // make sure result is always null terminated
// 
//   // convert application name from UTF-8 to Unicode
//   len = MultiByteToWideChar(CP_UTF8, 0, APP_NAME, -1, NULL, 0);
//   AUTO_STATIC_ARRAY(wchar_t, wAppName, 1024);
//   wAppName.Resize(len);
//   MultiByteToWideChar(CP_UTF8, 0, APP_NAME, -1, wAppName.Data(), len);
//   wAppName[len - 1] = 0; // make sure result is always null terminated
// 
//   if (!Silent) //when autotest is running, we want no error messages
//     MessageBoxW(NULL,wBuffer.Data(),wAppName.Data(),MB_OK|MB_ICONERROR);
//   SetCursor(LoadCursor(NULL,IDC_WAIT));
//   TerminateProcess(GetCurrentProcess(),1);
#else
  fputs(buf,stderr);
  fputc('\n',stderr);
#endif
}

#define ENABLE_WARNINGS 1
#define SINGLE_WARNING 1

static ErrorMessageLevel WarningLevel = EMNote;
//static RString WarningText;

ErrorMessageLevel GetMaxError()
{
  return WarningLevel;
}

// RString GetMaxErrorMessage()
// {
//   return WarningText;
// }

/*!
\patch 1.50 Date 4/11/2002 by Ondra
- Change: First warning during each mission is shown.
Only first warning after game was launched was shown before.
*/

void ResetErrors()
{
  WarningLevel = EMNote;
/*  WarningText = RString();*/
}

/*!
  \patch_internal 1.01 Date 06/21/2001 by Jirka
  - reimplementation of Warning message
  - if GWorld exist, display in game dialog with warning
  - otherwise, windows Message Box is performed
*/

void WarningMessageLevel( ErrorMessageLevel level, const char *format, va_list argptr)
{
//   BString<1024> buf;
//   vsprintf( buf, format, argptr );
// 
//   RptF("Warning Message: %s",(const char *)buf);
// 
// #if 1 // _ENABLE_REPORT
// 
//   #if SINGLE_WARNING
//     if( WarningLevel>=level ) return;
//     WarningLevel = level;
//   #endif
// 
//   static int avoidRecursion = 0;
//   if (avoidRecursion> 0) return;
// 
//   avoidRecursion++;
//   GDebugger.NextAliveExpected(15*60*1000);
// 
//   #if SINGLE_WARNING
//     WarningText = buf.cstr();
//   #endif
// 
//   // CHANGED in Patch 1.01
//   bool result = false;
//   // CHANGED in Patch 1.01
//   if (GWorld)
//   {
//     if (!Silent) //when autotest is running, we want no error messages
//       GWorld->CreateWarningMessage(buf.cstr());
//     result = true;
//   }
// 
//   // kill main application window
//   if (!result)
//   {
//         #if defined _WIN32 && !defined(_XBOX_ONE)
//     //if( HideCursor ) ShowCursor(TRUE);
//     DWORD icon = level<=EMWarning ? MB_ICONWARNING : MB_ICONERROR;
// 
//     // convert message from UTF-8 to Unicode
//     int len = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
//     AUTO_STATIC_ARRAY(wchar_t, wBuffer, 1024);
//     wBuffer.Resize(len);
//     MultiByteToWideChar(CP_UTF8, 0, buf, -1, wBuffer.Data(), len);
//     wBuffer[len - 1] = 0; // make sure result is always null terminated
// 
//     // convert application name from UTF-8 to Unicode
// 	len = MultiByteToWideChar(CP_UTF8, 0, APP_NAME, -1, NULL, 0);
//     AUTO_STATIC_ARRAY(wchar_t, wAppName, 1024);
//     wAppName.Resize(len);
// 	MultiByteToWideChar(CP_UTF8, 0, APP_NAME, -1, wAppName.Data(), len);
//     wAppName[len - 1] = 0; // make sure result is always null terminated
// 
//     if (!Silent) //when autotest is running, we want no error messages
//       MessageBoxW(NULL,wBuffer.Data(),wAppName.Data(),MB_OK|icon);
//     //if( HideCursor ) ShowCursor(FALSE);
//         #else
//         puts(buf);
//         #endif
//   }
//   // terminate
//   avoidRecursion--;
// #endif
}

void OFPFrameFunctions::WarningMessage( const char *format, va_list argptr)
{
  WarningMessageLevel(EMWarning,format,argptr);
}

/*!
\patch 1.50 Date 4/11/2002 by Ondra
- Changed: Error "No entry in config" no longer exits game.
This makes mission editing easier in case of misspelled class names.
*/

void OFPFrameFunctions::ErrorMessage(ErrorMessageLevel level, const char *format, va_list argptr)
{
  if (level<=EMError)
  {
    WarningMessageLevel(level,format,argptr);
  }
  else
  {
    ErrorMessage(format,argptr);
  }
}


#if _ENABLE_REPORT
  // always write to debug log

  #if _ENABLE_PERFLOG
    LogFile *GLogFile;

    static void ToDebugLog(const char *buf)
    {
      static bool open = false;
      static LogFile logFile;
      if (DisableLogs) return;
      if (!open || ResetLogFile)
      {
        char logFilename[MAX_PATH];
        if (GetLocalSettingsDir(logFilename))
        {
          CreateDirectory(logFilename, NULL);
          TerminateBy(logFilename, '\\');
        }
        else logFilename[0] = 0;
        strcat(logFilename, LogFilename);
        logFile.Open(logFilename);
        open = logFile.IsOpen();
        if (open) GLogFile = &logFile;
        ResetLogFile = false;
      }
      if (logFile.IsOpen())
        logFile.PrintF("%8.3f: %s",GlobalTickCount()*0.001,buf);
    }
  #endif


#if _ENABLE_REPORT
  // dirty hack: provide our own implementation of printf to allow using printf for debugging
  int __cdecl myprintf (const char *format, ...)
  {
    va_list argptr;
    va_start( argptr, format );
#if defined TRACE_ENABLED
    LogF(format, argptr);
#else
    CurrentAppFrameFunctions->LogF(format,argptr);
#endif
    va_end( argptr );
    return 1; // nobody really cares about number of chars
  }
#endif
/*!
\patch 5088 Date 11/15/2006 by Jirka
- Fixed: Unicode used for debugger output (OutputDebugString)
*/


# if defined TRACE_ENABLED
void OFPFrameFunctions::LogMessage(trace::level_t level, trace::context_t context, char const * file, int line, char const * fn, const char *format, va_list argptr)
{
  if (DisableLogs) return;
  trace::WriteVA(level, context, file, line, fn, format, argptr);
}
# else
void OFPFrameFunctions::LogF( const char *format, va_list argptr)
{
  if (DisableLogs) return;
  char buf[4096];
	char output[4096];
/*  LString output = buf;*/
  #ifdef _WIN32
//   if (GDebugger.IsDebugger())
//   {
//     // in debugger we want to see the timestamps
// 
//     int printed = sprintf(buf,"%s:%8.3f: ",cc_cast(InstanceName),GlobalTickCount()*0.001f);
// 
//     output = buf + printed;
//   }
  #endif

  vsprintf(output,format,argptr);

	// make sure backslash is always present, but if it already ends with one, do not add another one
  const char *eol = strrchr(buf,'\n');

	// if not there or not at the end, append it
  if (!eol || eol[1]!=0)
  {
    strcat(buf,"\n");
  }

  // assume someone will catch it even when there is no debugger
  #ifdef _WIN32

  // convert from UTF-8 to Unicode
  int len = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
  const int maxLen = 2048;
  wchar_t wBuffer[2048];
  if (len>maxLen-1) len = maxLen-1;
  MultiByteToWideChar(CP_UTF8, 0, buf, -1, wBuffer, len);
  wBuffer[len - 1] = 0; // make sure result is always null terminated
  OutputDebugStringW(wBuffer);

  #elif defined(ENF_PS4)
    printf("%s", buf.cstr());

  #endif

  #if _ENABLE_PERFLOG
    ToDebugLog(buf);
  #endif
}
# endif

#endif  // of ENABLE_REPORT

#if !_ENABLE_REPORT || !defined TRACE_ENABLED
void OFPFrameFunctions::ErrF( const char *format, va_list argptr)
{
//   // writing to report is very slow - if it happens, we want to see it in stats
//   PROFILE_SCOPE(rpt)
// 
//   BString<512> buf;
//   vsprintf(buf,format,argptr);
// 
// #if _ENABLE_REPORT
//   if (GDebugger.IsDebugger())
//   {
//     // avoid using OFPFrameFunctions::LogF to prevent time-stamping asserts (we want IDE to catch them)
//     strcat(buf,"\n");
// #ifdef _WIN32
//     OutputDebugString(buf);
// #else
//     fputs(buf,stderr);
// #endif
//     #if _ENABLE_PERFLOG
//       ToDebugLog(buf);
//     #endif
// #if defined(_WIN32) && _RELEASE
// 			__debugbreak(); // to disable debugbreak in runtime in watch write " *(char*) eip " (rip for x64) and change value from int 3 "-52" to nop "144"or"-112"
// #endif
//   }
//   else
// #endif
//   {
// #if _RELEASE
// #ifdef _WIN32
//     //::RptF("%s",(const char *)buf);
//     //DebugExceptionTrap::LogFFF(buf);
// #else
//     fputs(buf,stderr);
//     fputc('\n',stderr);
// #endif
// #endif
// 	}
}
#endif

/*!
\patch 5262 Date 8/7/2008 by Bebul
- New: Optional server.cfg entry timeStampFormat to specify time stamp for each line in *.rpt file.
  Possible values: "none", "short", "full".
*/

// TimeStampFormat GTimeStampFormat = TSFNone;
// 
// void InitLstFBuf(/*OUT*/BString<1024> &buf, const char *format, va_list argptr )
// {
// #if _ENABLE_DEDICATED_SERVER
//   switch (GTimeStampFormat)
//   {
//   case TSFFull:
//     {
// #ifdef _WIN32
//       SYSTEMTIME syst;
//       GetLocalTime(&syst);
//       sprintf(buf, "%4d/%02d/%02d, %2d:%02d:%02d ",
//         syst.wYear,syst.wMonth,syst.wDay,
//         syst.wHour,syst.wMinute,syst.wSecond
//         );
// #else
//       time_t now;
//       time(&now);
//       tm *t = localtime(&now);
//       sprintf(buf, "%4d/%02d/%02d, %2d:%02d:%02d ",
//          t->tm_year+1900, t->tm_mon+1, t->tm_mday,
//          t->tm_hour,t->tm_min,t->tm_sec);
// #endif
//       BString<1024> buf2;
//       vsprintf(buf2,format,argptr);
//       buf += buf2;
//     }
//     break;
//   case TSFShort:
//     {
// #ifdef _WIN32
//       SYSTEMTIME syst;
//       GetLocalTime(&syst);
//       sprintf(buf, "%2d:%02d:%02d ", syst.wHour, syst.wMinute, syst.wSecond);
// #else
//       time_t now;
//       time(&now);
//       tm *t = localtime(&now);
//       sprintf(buf, "%2d:%02d:%02d ", t->tm_hour,t->tm_min,t->tm_sec);
// #endif
//       BString<1024> buf2;
//       vsprintf(buf2,format,argptr);
//       buf += buf2;
//     }
//     break;
//   case TSFNone:
//     vsprintf(buf,format,argptr);
//     break;
//   }
// #else
//   vsprintf(buf,format,argptr);
// #endif
// }
// 
// void FinishLstFBuf(BString<1024> &buf)
// {
//   if (DisableLogs) return;
// #if _ENABLE_REPORT
//   if (GDebugger.IsDebugger())
//   {
//     // avoid using OFPFrameFunctions::LogF to prevent time-stamping asserts (we want IDE to catch them)
//     strcat(buf,"\n");
// #   if !defined TRACE_ENABLED
//     OutputDebugString(buf);
// #endif
//     #if _ENABLE_PERFLOG
//     ToDebugLog(buf);
//     #endif
//   }
//   else
// #endif
//   {
// #if _RELEASE
// # ifdef _WIN32
// #   if _ENABLE_REPORT
//     LogF("%s",(const char *)buf);
// #   endif
// # else
//     fputs(buf,stderr);
//     fputc('\n',stderr);
// # endif
// #endif
//   }
// }
// 
void OFPFrameFunctions::LstF( const char *format, va_list argptr)
{
//   if (DisableLogs) return;
//   // writing to report is very slow - if it happens, we want to see it in stats
//   PROFILE_SCOPE(rpt)
// 
//   BString<1024> buf;
//   InitLstFBuf(buf, format, argptr);
// 
// #if _RELEASE
//   #if defined _WIN32 && !defined _XBOX_ONE
//   //OutputDebugString(buf);OutputDebugString("\r\n");
//   DebugExceptionTrap::LogFFF(buf,false);
//   #endif
// #endif
// # if defined TRACE_ENABLED
//   trace::WriteVA(LL_DEBUG, CTX_DEFAULT, __FILE__, __LINE__, __FUNCTION__, format, argptr);
// # endif
//   FinishLstFBuf(buf);
}

void OFPFrameFunctions::LstFDebugOnly( const char *format, va_list argptr)
{
//   if (DisableLogs) return;
//   // writing to report is very slow - if it happens, we want to see it in stats
//   PROFILE_SCOPE(rpt)
// 
//     BString<1024> buf;
//   InitLstFBuf(buf, format, argptr);
// # if defined TRACE_ENABLED
//   trace::WriteVA(LL_VERBOSE, CTX_DEFAULT, __FILE__, __LINE__, __FUNCTION__, format, argptr);
// # endif
//   FinishLstFBuf(buf);
}


