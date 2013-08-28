#pragma once
#include <CryCommon/platform.h>
#include <CryCommon/ILog.h>
#include "trace.h"

class EngineLogCallback : public ILogCallback
{
	virtual TRACE_API void OnWriteToConsole (const char * msg, bool);
	virtual TRACE_API void OnWriteToConsole (char const * file, int line, const char * msg, bool);
	virtual TRACE_API void OnWriteToFile (const char * msg, bool);
};

#include <CryCommon/FrameProfiler.h>
struct EngineProfileCallback : IFrameProfileSystem
{
	EngineProfileCallback (IFrameProfileSystem * orig) : m_original(orig), m_frame_id(0) { }
	EngineProfileCallback () : m_original(0), m_frame_id(0) { }
	IFrameProfileSystem * m_original;
	unsigned long long m_frame_id;

	virtual TRACE_API void Reset ();					//! Reset all profiling data.
	virtual TRACE_API void AddFrameProfiler (CFrameProfiler * pProfiler); //! Add new frame profiler.
	
	virtual TRACE_API void StartFrame ();				//! Must be called at the start of the frame.
	virtual TRACE_API void EndFrame ();				//! Must be called at the end of the frame.
	
	virtual TRACE_API void OnSliceAndSleep ();		//! Must be called when something quacks like the end of the frame.

	virtual TRACE_API int GetProfilerCount () const;	//! Get number of registered frame profilers.
	virtual TRACE_API CFrameProfiler * GetProfiler (int index) const; //! Get frame profiler at specified index. @param index must be 0 <= index < GetProfileCount() 
	
	virtual TRACE_API int GetPeaksCount () const;		//! Get number of registered peak records.
	virtual TRACE_API SPeakRecord const * GetPeak (int index) const; //! Get peak record at specified index.  @param index must be 0 <= index < GetPeaksCount() 
	virtual TRACE_API CFrameProfilerSection const * GetCurrentProfilerSection ();//! Gets the bottom active section.

	virtual TRACE_API void Enable (bool bCollect, bool bDisplay);
	virtual TRACE_API void SetSubsystemFilter (bool bFilterSubsystem, EProfiledSubsystem subsystem);
	virtual TRACE_API bool IsEnabled () const;		//! True if profiler is turned off (even if collection is paused).
	virtual TRACE_API bool IsVisible () const;		//! True if profiler statistics is visible
	virtual TRACE_API bool IsProfiling () const;	//! True if profiler must collect profiling data.
	virtual TRACE_API void SetDisplayQuantity (EDisplayQuantity quantity);

	// For custom frame profilers.
	virtual TRACE_API void StartCustomSection (CCustomProfilerSection * pSection);
	virtual TRACE_API void EndCustomSection (CCustomProfilerSection * pSection);

	//! Register peak listener callback to be called when peak value is greater then this.
	virtual TRACE_API void AddPeaksListener (IFrameProfilePeakCallback * pPeakCallback);
	virtual TRACE_API void RemovePeaksListener (IFrameProfilePeakCallback * pPeakCallback);

	//! access to call stack string
	virtual TRACE_API char const * GetFullName (CFrameProfiler * pProfiler);
	virtual TRACE_API char const * GetModuleName (CFrameProfiler * pProfiler);
};
