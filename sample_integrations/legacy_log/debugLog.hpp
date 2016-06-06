#ifdef _MSC_VER
#pragma once
#endif

#ifndef _DEBUGLOG_HPP
#define _DEBUGLOG_HPP

#include <Es/essencepch.hpp>


// critical error - terminate application
void CCALL ErrorMessage( const char *format, ... );

// noncritical error - may terminate application
void CCALL WarningMessage( const char *format, ... );

// assertion failed
void CCALL FailHook( const char *text );

#ifdef _MSC_VER
  #if _MANAGED
    using namespace System::Diagnostics;
    #define BreakIntoDebugger(text) Debugger::Break()
  #elif _MSC_VER>=1300
    #define BreakIntoDebugger(text) __debugbreak()
  #else
    // x86 specific break code - for older compilers not supporting __debugbreak()
    #define BreakIntoDebugger(text) __asm {int 3}
  #endif
#else
  #define BreakIntoDebugger(text)
#endif

  /// check if calling from the main thread
  bool CheckMainThread();
  /// check that calling thread has not changed
  bool CheckSameThread(int &id);

#if _ENABLE_REPORT
  /// trigger breakpoint (even when no debugger is attached)
  #define FailHookCritical(text) BreakIntoDebugger(text)
  /// assert when called from different thread than the main one
  #define AssertMainThread() DoAssertCritical( ::CheckMainThread() )
  /// assert when called multiple times from different thread
  #define AssertSameThread(id) DoAssertCritical( ::CheckSameThread(id) )
#else
  #define AssertMainThread()
  #define AssertSameThread(id)
  #define FailHookCritical(text)
#endif

#if defined(_MSC_VER) && _MSC_VER>=1300
  #define NoLog __noop
#else
  #define NoLog (void)
#endif

  #if NDEBUG
	#define FailHook(text) BreakIntoDebugger(text)
  #else
  #define FailHook(text) BreakIntoDebugger(text)
  #endif

#ifndef _ENABLE_REPORT
  // assume default value true
  #pragma message("_ENABLE_REPORT should be defined before using debugLog.hpp")
  #pragma message("This is normally done by Es/essencePch.hpp")
  #define _ENABLE_REPORT 1
#endif

  #define DoVerify2(expr, desc) \
  { \
    if( !(expr) ) \
    { \
      ErrF("%s(%d) : Assertion failed '%s', '%s'",__FILE__,__LINE__,#expr,desc); \
      FailHook(#expr); \
    } \
  }
  #define DoAssert2(expr, desc) DoVerify2(expr, desc)
#if _ENABLE_REPORT
  #define DoVerify(expr) \
  { \
    if( !(expr) ) \
    { \
      ErrF("%s(%d) : Assertion failed '%s'",__FILE__,__LINE__,#expr); \
      FailHook(#expr); \
    } \
  }
  #define DoAssert(expr) DoVerify(expr)
 #define DoAssertCritical(expr) \
  { \
    if( !(expr) ) \
    { \
      RptF("%s(%d) : Assertion failed '%s'",__FILE__,__LINE__,#expr); \
      FailHookCritical(#expr); \
    } \
  }
# if defined TRACE_ENABLED
# else
  void PUBLIC_API LogF( const char *format, ... );
# endif
#else
  #define DoVerify(expr) (expr)
  #define DoAssert(expr) (false ? (void)(expr) : (void)0)
  #define DoAssertCritical(expr)
  #define LogF NoLog
#endif

  void LogFForced(float delay, char const *text, ...);


#if _ENABLE_REPORT
# if defined TRACE_ENABLED
# else
  void PUBLIC_API  ErrF( const char *format, ... ); // does produce callstack report
  void PUBLIC_API  LstF( const char *format, ... ); // no call stack report
# endif
#else
  void PUBLIC_API  ErrF( const char *format, ... ); // does produce callstack report
  void PUBLIC_API  LstF( const char *format, ... ); // no call stack report
#endif

// log to debugger output, even in retail version
void CCALL LogDebugger( const char *format, ... ); // no call stack report

#if defined TRACE_ENABLED
# ifdef _DEBUG
#   define AssertDebug( expr ) RvVerify(expr)
# else
#   define AssertDebug( expr ) (false ? ((void)(expr)) : (void)0)
# endif
# if _ENABLE_REPORT
#   define RptF ::LogF
# else
#   define RptF LstF
# endif
#else
# ifdef _DEBUG
#   define AssertDebug( expr ) RvVerify(expr)
#   define ErrF ErrF
#   define RptF LogF
# else
#   define AssertDebug( expr ) (false ? ((void)(expr)) : (void)0)
#   define ErrF ErrF
#   define RptF LstF
# endif
#endif

#if defined NDEBUG && !defined _X1_TEMP
  #define Assert( expr ) (false ? ((void)(expr)) : (void)0)
	#define Assert2(exp, desc)
  #define RlsAssert(exp) DoAssert(exp)
  #define RlsAssert2(exp, desc) DoAssert2(exp, desc)
#define RvVerify( expr ) (expr)
#if _SUPER_RELEASE
  #define Fail(text) ErrF("%s",text);
#else
  #define Fail(text) ErrF("%s(%d) : %s",__FILE__,__LINE__,text);
#endif
  #define Log NoLog
#else
  #define Assert( expr ) DoAssert(expr)
	#define Assert2(expr, desc) DoAssert2(expr, desc)
  #define RlsAssert(exp) DoAssert(exp)
  #define RlsAssert2(exp, desc) DoAssert2(exp, desc)
	#define RvVerify( expr ) DoAssert(expr)
  #define Fail(text) {ErrF("%s(%d) : %s",__FILE__,__LINE__,text);FailHook(text);}
  #define Log ::LogF
#endif

#include <stdio.h>

#pragma warning(disable:4996)
inline const char *FileLineF( const char *file, int line, const char *postfix )
{
  static char buf[512];
  #if (defined __GNUC__ || defined __INTEL_COMPILER)
  sprintf(buf,"%s(%d): %s",file,line,postfix);
  #else
  snprintf(buf,sizeof(buf),"%s(%d): %s",file,line,postfix);
  #endif
  buf[sizeof(buf)-1]=0;
  return buf;
}

#define FileLine(postfix) FileLineF(__FILE__,__LINE__,postfix)

// object instance counting

#if _DEBUG || _PROFILE
  #define COUNT_CLASS_INSTANCES 1
#else
  #define COUNT_CLASS_INSTANCES 0
#endif

#if COUNT_CLASS_INSTANCES

class AllCountersLink;

extern PUBLIC_API AllCountersLink *AllCountInstances[1024];
extern PUBLIC_API int AllCountInstancesCount;

/// an item listed in the AllCountInstances, corresponds to a counted type
class PUBLIC_API AllCountersLink
{
  int _counter;

  public:
//	ENF_DECLARE_MMOBJECT(AllCountersLink);

  AllCountersLink()
  {
    _counter = 0;
    if (AllCountInstancesCount>lenof(AllCountInstances))
    {
      Fail("AllCountInstances overflow");
      return;
    }
    AllCountInstances[AllCountInstancesCount++] = this;
  }

  ~AllCountersLink(){}

  void operator ++ (int) {_counter++;}
  void operator -- (int) {_counter--;}
  int GetValue() const {return _counter;}
};

//! helper base class performing instance counting
/** Id is here to make sure each class has its own instance of CountInstances */
template <class Id>
class PUBLIC_API CountInstances
{
  static AllCountersLink _allCountersLink;

  public:
  CountInstances() {_allCountersLink++;}
  CountInstances(const CountInstances &src) {_allCountersLink++;}
  ~CountInstances() {_allCountersLink--;}

  /// get number of instances
  static int GetInstanceCount() {return _allCountersLink.GetValue();}
};

#ifdef ENF_BUILD

template <class Id>
AllCountersLink CountInstances<Id>::_allCountersLink;

#endif



/*
// example usage:

class ClassA:
  public BaseA, public BaseB, public CountInstances<ClassA>
*/

#else

// dummy implementation - will be effectively removed by the compiler
template <class Id>
class PUBLIC_API CountInstances {};

#endif

template <class Type, class SourceType>
struct CheckCast
{
  static void Check(SourceType item)
  {
    Assert(!item || dynamic_cast<Type>(item));
  }
};

/// partial specialization used to derive a pointer type from a reference type
template <class Type, class SourceType>
struct CheckCast<Type &,SourceType &>
{
  static void Check(SourceType &item)
  {
    Assert(dynamic_cast<Type *>(&item));
  }
};

/// perform a static_cast asserted by a dynamic_cast
template <class Type, class SourceType>
__forceinline Type static_cast_checked(SourceType *item)
{
  CheckCast<Type,SourceType *>::Check(item);
  return static_cast<Type>(item);
}

/// overload for references
template <class Type, class SourceType>
__forceinline Type static_cast_checked(SourceType &item)
{
  CheckCast<Type,SourceType &>::Check(item);
  return static_cast<Type>(item);
}

#endif
