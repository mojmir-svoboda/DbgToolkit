// binarize.cpp : Defines the entry point for the console application.
//
#include "lib/wpch.hpp"

#if defined(ENF_PS4)
// mk:TODO PS4: Binarizer
bool isBinarizer = false;

#elif _ENABLE_SINGLE_BINARIZE_EXECUTABLE

#include "../posAccess/posAccess.hpp"
#include "../lib/Shape/shape.hpp"
#include "../lib/Shape/modelConfig.hpp"
#include "../lib/landscape.hpp"
#include "../lib/engine.hpp"
#include "../lib/engDummy.hpp"
#include "../lib/world.hpp"
#include <El/FileServer/fileServer.hpp>
#include "../lib/depMake.hpp"
#include "../lib/rtAnimation.hpp"
#include "../lib/keyInput.hpp"

#include <Es/Files/fileNames.hpp>
#include <Es/Files/fileContainer.hpp>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <Es/common/win.h>
#include <Es/Containers/rStringArray.hpp>
#include <El/FileBinMake/fileBinMake.hpp>
#include <El/paramfile/paramfile.hpp>
#include "../lib/appFrameExt.hpp"
#include "../lib/Network/network.hpp"

#include "system/enf_vfilesystem.h"
#include "common/enf_staticbuffer.h"
#include "script/enf_scriptapi.h"
#include "lib/GameLib/Core/EnforceAPI.h"

class PUBLIC_API BinarizeFrameFunctions : public OFPFrameFunctions
{
public:
	ENF_DECLARE_MMOBJECT(BinarizeFrameFunctions);

  BinarizeFrameFunctions() : OFPFrameFunctions() {}
  void ErrorMessage(const char *format, va_list argptr) override;
  void WarningMessage( const char *format, va_list argptr) override;
  void ErrorMessage(ErrorMessageLevel level, const char *format, va_list argptr) override;
#if _ENABLE_REPORT && !defined TRACE_ENABLED
  void LogF( const char *format, va_list argptr) override;
  void ErrF( const char *format, va_list argptr) override;
#endif
  void LstFDebugOnly(const char *format, va_list argptr) override;
  void LstF( const char *format, va_list argptr) override;
};

void BinarizeFrameFunctions::ErrorMessage(const char *format, va_list argptr)
{
  RptF(format,argptr);
  #if _DEBUG
    BreakIntoDebugger("");
  #endif
}

void BinarizeFrameFunctions::WarningMessage( const char *format, va_list argptr)
{
  RptF(format,argptr);
  #if _DEBUG
    BreakIntoDebugger("");
  #endif
}

void BinarizeFrameFunctions::ErrorMessage(ErrorMessageLevel level, const char *format, va_list argptr)
{
  RptF(format,argptr);
  #if _DEBUG
    //BreakIntoDebugger();
  #endif
}

#if _ENABLE_REPORT && !defined TRACE_ENABLED
void BinarizeFrameFunctions::LogF( const char *format, va_list argptr)
{
  BString<512> buf;

  vsprintf(buf,format,argptr);
  strcat(buf,"\n");
  OutputDebugString(buf);
  //printf(buf);
}
void BinarizeFrameFunctions::ErrF( const char *format, va_list argptr)
{
  BString<512> buf;

  vsprintf(buf,format,argptr);
  strcat(buf,"\n");
  OutputDebugString(buf);
  fprintf(stderr,buf);
}
#endif

void BinarizeFrameFunctions::LstF( const char *format, va_list argptr)
{
  BString<512> buf;

  vsprintf(buf,format,argptr);
  strcat(buf,"\n");
  OutputDebugString(buf);
  fprintf(stderr,buf);
}

void BinarizeFrameFunctions::LstFDebugOnly(const char *format, va_list argptr)
{
  LstF(format, argptr);
}

static BinarizeFrameFunctions GBinarizeFrameFunctions INIT_PRIORITY_URGENT;

extern ParamFile Pars;
extern bool GShrinkP3D;

void NormalizePathRString(RString& path)
{
  int length = path.GetLength();
  if (0 == length)
    return;

  char *str = new char[path.GetLength()+1];
  strcpy(str, path.Data());
  NormalizePath(str);
  path = str;
  delete[] str;
}

const char *GetCWD()
{
  static char cwd[1024];
  getcwd(cwd,sizeof(cwd));
  return cwd;
}

static void MakeAllDirs(const char *path)
{
  const char *delim = strchr(path,':');
  if (!delim) delim = path;
  delim = strchr(delim+1,'\\');
  do
  {
    char parent[1024];
    strncpy(parent,path,delim-path);
    parent[delim-path] = 0;
    mkdir(parent);
    delim = strchr(delim+1,'\\');
  } while(delim);
}

static const char *RestArg(const char *a, const char *b)
{
  int l = (int)strlen(b);
  if (strnicmp(a,b,l)) return NULL;
  return a+l;
}

static time_t FileTimestamp(const char *name)
{
  return QFBankQueryFunctions::TimeStamp(name);
}


bool isBinarizer = false;
int main(int argc, char* argv[])
{
	isBinarizer = true;
  // redirect AppFrameFunctions to console stdout,stderr
  CurrentAppFrameFunctions = &GBinarizeFrameFunctions;
	RptF("Binarize started\n");
}