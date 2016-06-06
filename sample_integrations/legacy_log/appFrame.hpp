#ifdef _MSC_VER
#pragma once
#endif

#ifndef __APP_FRAME_HPP
#define __APP_FRAME_HPP

#include <stdarg.h>
#include "cfg.h"
//#include <Es/Strings/rString.hpp>
//#include <El/Color/colorsFloat.hpp>


#ifndef _ENABLE_REPORT
#error _ENABLE_REPORT must be defined when appFrame is used
#endif

// Application framework

//! error level
enum ErrorMessageLevel
{
	//! may be ignored, marginal impact (like only slight performance degradation)
	EMNote,
  //! some data are missing with no great impact on gameplay
  EMMissingData,
	//! application is able to continue, but with limited functionality
	EMWarning,
	//! application is not able to perform requested task, but is able to continue
	EMError,
	//! application is not able to continue
	EMCritical,
	//! no error level - used to disable all errors
	EMDisableAll
};

#if _ENABLE_REPORT && !defined TRACE_ENABLED
void PUBLIC_API LogF(const char *format,...);
void vaLogF(const char *format, va_list argptr);
#endif
void PUBLIC_API LstF(const char *format,...);
void vaLstF(const char *format, va_list argptr);
#if !defined TRACE_ENABLED
void PUBLIC_API ErrF(const char *format,...);
#endif
void vaErrF(const char *format, va_list argptr);

void LogDebugger(const char *format, va_list argptr);

void ErrorMessage(ErrorMessageLevel level, const char *format,...);
void ErrorMessage(const char *format,...);
void WarningMessage(const char *format,...);

bool CheckMainThread(int id);
bool CheckMainThread();
bool CheckSameThread(int &id);
void ResetMainThread();

void GlobalShowMessage(int timeMs, const char *msg, ...);
void GlobalDiagMessage(int &handle, int id, int timeMs, const char *msg, ...);
//void GlobalDiagMessagePos(int x, int y, const PackedColor& col, int maxChars, const char *msg, ...);
//void GlobalShowMessage(int timeMS, const char *msg, InitVal<int,-1> *handles, int n);

#if _ENABLE_REPORT
	// use Format function to preformat if necessary
	#define DIAG_MESSAGE(time,...) do \
	{	\
		static int handle=-1; \
		GlobalDiagMessage(handle, 0, time, __VA_ARGS__); \
	} while (false);
	#define DIAG_MESSAGE_ID(time,id,...) do \
	{	\
		static int handle=-1; \
		GlobalDiagMessage(handle, id, time, __VA_ARGS__); \
	} while (false);
	#define DIAG_MESSAGE_POS(x, y, color, maxChars, ...) \
		GlobalDiagMessagePos(x, y, color, maxChars, __VA_ARGS__);

#else
	#define DIAG_MESSAGE(time,...) do {} while (false);
	#define DIAG_MESSAGE_ID(time,id,...) do {} while (false);
	#define DIAG_MESSAGE_POS(x, y, color, maxChars, ...)
#endif
// Default implementation

class PUBLIC_API AppFrameFunctions
{
public:
	//ENF_DECLARE_MMOBJECT(AppFrameFunctions);
	AppFrameFunctions() {};
	virtual ~AppFrameFunctions() {};

#if _ENABLE_REPORT
# if defined TRACE_ENABLED
  virtual void LogMessage(trace::level_t level, trace::context_t context, char const * file, int line, char const * fn, const char *format, va_list argptr);
# else
	virtual void LogF(const char *format, va_list argptr);
	virtual void ErrF(const char *format, va_list argptr);
# endif
#else
	virtual void ErrF(const char *format, va_list argptr);
#endif
	virtual void LstF(const char *format, va_list argptr);
  virtual void LstFDebugOnly(const char *format, va_list argptr);
	virtual void LogDebugger(const char *format, va_list argptr);

	virtual void ErrorMessage(ErrorMessageLevel level, const char *format, va_list argptr)
  {
#if _ENABLE_REPORT
# if defined TRACE_ENABLED
    trace::level_t l = LL_NORMAL;
    switch (level)
    {
      case EMNote: l = LL_NORMAL; break;
      case EMMissingData: l = LL_ERROR; break;
      case EMWarning: l = LL_WARNING; break;
      case EMError: l = LL_ERROR; break;
      case EMCritical: l = LL_FATAL; break;
      case EMDisableAll: l = LL_NORMAL; break;
      default: l = LL_NORMAL; break;
    }
    LogMessage(l, CTX_LEGACY, __FILE__, __LINE__, __FUNCTION__, format,argptr);
# else
    LogF(format,argptr);
#endif
#endif
  }
	virtual void ErrorMessage(const char *format, va_list argptr)
  {
#if _ENABLE_REPORT
# if defined TRACE_ENABLED
    LogMessage(LL_ERROR, CTX_LEGACY, __FILE__, __LINE__, __FUNCTION__, format,argptr);
# else
    LogF(format,argptr);
#endif
#endif
  }
	virtual void WarningMessage(const char *format, va_list argptr)
  {
#if _ENABLE_REPORT
# if defined TRACE_ENABLED
    LogMessage(LL_WARNING, CTX_LEGACY, __FILE__, __LINE__, __FUNCTION__, format,argptr);
# else
    LogF(format,argptr);
#endif
#endif
  }
	
	virtual void ShowMessage(int timeMs, const char *msg) {}
	virtual void DiagMessage(int &handle, int id, int timeMs, const char *msg, ...) {}
// 	virtual void DiagMessage(int x, int y, const PackedColor& col, int maxChars, const char *msg, ...) {}
//   virtual RString GetAppCommandLine() const {return RString();}

  //@{ PIX/D3D/graphical profiling interface
  virtual int ProfileBeginGraphScope(unsigned int color,const char *name) const {return 0;}
  virtual int ProfileEndGraphScope() const {return 0;}
  //@}

  /// check if given thread is the main thread
  virtual bool CheckMainThread(int threadId) const;
  /// called to check if executing from different than the main thread
  virtual bool CheckMainThread() const;
  virtual bool CheckSameThread(int &id) const;
  virtual void ResetMainThread();
  virtual int GetMainThreadId() const;
};

extern AppFrameFunctions *CurrentAppFrameFunctions;

#endif

