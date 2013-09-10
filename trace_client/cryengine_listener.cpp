#include "cryengine_listener.h"

#include <CryCommon/platform.h>
#include <CryCommon/platform_impl.h>
#include <CryCommon/ILog.h>

using namespace trace;

#if defined TRACE_ENABLED
namespace {
	struct CallbackSplitInfo
	{
		char const *	m_Substring;
		E_TraceLevel	m_TraceLevel;
	};
	// Here are rules for assigning trace level according to what is in string coming from CryEngine.
	// If a rule is valid, the next rules are not evaluated
	static CallbackSplitInfo s_CallbackSplitInfos[] =
	{
		{"Error parsing <", e_Error},
		{"[Error]", e_Error},
		{"[Lua Error]", e_Error},
		{"[Warning]", e_Warning},
	};
}
#endif

void EngineLogCallback::OnWriteToConsole (char const * msg, bool b)
{
	OnWriteToConsole(__FILE__, __LINE__, msg, b);
}

void EngineLogCallback::OnWriteToConsole (char const * file, int line, char const * msg, bool)
{
#if defined TRACE_ENABLED
	E_TraceLevel traceLevel = e_Info;
	for (int i = 0; i < sizeof(s_CallbackSplitInfos)/sizeof(CallbackSplitInfo); ++i)
	{
		if (strstr(msg, s_CallbackSplitInfos[i].m_Substring))
		{
			traceLevel = s_CallbackSplitInfos[i].m_TraceLevel;
			break;
		}
	}
	trace::Write(traceLevel, trace::CTX_Engine, file, line, __FUNCTION__, "%s", msg);
#else
	WH_UNUSED_VARIABLE(msg);
#endif
}

void EngineLogCallback::OnWriteToFile (char const * , bool) { }


/// profiler
void EngineProfileCallback::Reset ()
{
	m_frame_id = 0;
	m_original->Reset();
}
void EngineProfileCallback::AddFrameProfiler (CFrameProfiler * pProfiler)
{
	m_original->AddFrameProfiler(pProfiler);
}

void EngineProfileCallback::StartFrame ()
{
	++m_frame_id;
	//TRACE_GANTT_FRAME_BGN(trace::e_Info, trace::CTX_Engine, "profile/g0/frame %i...", m_frame_id);
	m_original->StartFrame();
}
void EngineProfileCallback::EndFrame ()
{
	//TRACE_GANTT_FRAME_END(trace::e_Info, trace::CTX_Engine, "aa0/g0/frame %i", m_frame_id);
	m_original->EndFrame();
}

void EngineProfileCallback::OnSliceAndSleep ()
{
	m_original->OnSliceAndSleep();
}

int EngineProfileCallback::GetProfilerCount () const
{
	return m_original->GetProfilerCount();
}
CFrameProfiler * EngineProfileCallback::GetProfiler (int index) const
{
	return m_original->GetProfiler(index);
}

int EngineProfileCallback::GetPeaksCount () const
{
	return m_original->GetPeaksCount();
}
SPeakRecord const * EngineProfileCallback::GetPeak (int index) const
{
	return m_original->GetPeak(index);
}
CFrameProfilerSection const * EngineProfileCallback::GetCurrentProfilerSection ()
{
	return m_original->GetCurrentProfilerSection();
}

void EngineProfileCallback::Enable (bool bCollect, bool bDisplay)
{
	m_original->Enable(bCollect, bDisplay);
}
void EngineProfileCallback::SetSubsystemFilter (bool bFilterSubsystem, EProfiledSubsystem subsystem)
{
	m_original->SetSubsystemFilter(bFilterSubsystem, subsystem);
}
bool EngineProfileCallback::IsEnabled () const
{
	return m_original->IsEnabled();
}
bool EngineProfileCallback::IsVisible () const
{
	return m_original->IsVisible();
}
bool EngineProfileCallback::IsProfiling () const
{
	return m_original->IsProfiling();
}
void EngineProfileCallback::SetDisplayQuantity (EDisplayQuantity quantity)
{
	return m_original->SetDisplayQuantity(quantity);
}

void EngineProfileCallback::StartCustomSection (CCustomProfilerSection * pSection)
{
	m_original->StartCustomSection(pSection);
}
void EngineProfileCallback::EndCustomSection (CCustomProfilerSection * pSection)
{
	m_original->EndCustomSection(pSection);
}

void EngineProfileCallback::AddPeaksListener (IFrameProfilePeakCallback * pPeakCallback)
{
	m_original->AddPeaksListener(pPeakCallback);
}
void EngineProfileCallback::RemovePeaksListener (IFrameProfilePeakCallback * pPeakCallback)
{
	m_original->RemovePeaksListener(pPeakCallback);
}

char const * EngineProfileCallback::GetFullName (CFrameProfiler * pProfiler)
{
	return m_original->GetFullName(pProfiler);
}
char const * EngineProfileCallback::GetModuleName (CFrameProfiler * pProfiler)
{
	return m_original->GetModuleName(pProfiler);
}
