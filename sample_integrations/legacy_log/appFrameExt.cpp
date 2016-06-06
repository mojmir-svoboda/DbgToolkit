//#include "lib/wpch.hpp"
#include "appFrameExt.hpp"


// RString OFPFrameFunctions::GetAppCommandLine() const
// {
// #if defined (_WIN32)
// 	return ::GetCommandLine();
// #else
// 	return RString();
// #endif
// }

int OFPFrameFunctions::ProfileBeginGraphScope(unsigned int color, const char *name) const
{
// 	if (GEngine)
// 	{
// 		return GEngine->ProfileBeginGraphScope(color, name);
// 	}
	return 0;
}
int OFPFrameFunctions::ProfileEndGraphScope() const
{
// 	if (GEngine)
// 	{
// 		return GEngine->ProfileEndGraphScope();
// 	}
	return 0;
}

void OFPFrameFunctions::ShowMessage(int timeMs, const char *msg)
{
// 	if (!GEngine) return;
// 
// 	static int handle = -1;
// 	DiagMessage(handle, 0, timeMs, msg);
}