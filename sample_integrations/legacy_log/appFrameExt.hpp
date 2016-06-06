#ifdef _MSC_VER
#pragma once
#endif

#ifndef __APP_FRAME_EXT_HPP
#define __APP_FRAME_EXT_HPP

#include "appFrame.hpp"

class PUBLIC_API OFPFrameFunctions : public AppFrameFunctions
{
protected:

public:
/*	ENF_DECLARE_MMOBJECT(OFPFrameFunctions);*/

#if _ENABLE_REPORT
# if defined TRACE_ENABLED
  virtual void LogMessage(trace::level_t level, trace::context_t context, char const * file, int line, char const * fn, const char *format, va_list argptr);
# else
  virtual void LogF(const char *format, va_list argptr);
  virtual void ErrF(const char *format, va_list argptr);
# endif
#else // super release
  virtual void ErrF(const char *format, va_list argptr);
#endif
  virtual void LstF(const char *format, va_list argptr);
  virtual void LstFDebugOnly(const char *format, va_list argptr);

  virtual void ErrorMessage(const char *format, va_list argptr);
  virtual void ErrorMessage(ErrorMessageLevel level, const char *format, va_list argptr);
  virtual void WarningMessage(const char *format, va_list argptr);

  virtual void ShowMessage(int timeMs, const char *msg);
//   virtual void DiagMessage(int &handle, int id, int timeMs, const char *msg, ...);
//   virtual void DiagMessage(int x, int y, const PackedColor& col, int maxChars, const char *msg, ...);
//   virtual RString GetAppCommandLine() const;
  virtual int ProfileBeginGraphScope(unsigned int color,const char *name) const;
  virtual int ProfileEndGraphScope() const;
};

#endif
