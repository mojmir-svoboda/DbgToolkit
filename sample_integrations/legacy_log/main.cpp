#include "appFrame.hpp"
#include "appFrameExt.hpp"
static OFPFrameFunctions GOFPFrameFunctions INIT_PRIORITY_URGENT;
AppFrameFunctions *CurrentAppFrameFunctions = &GOFPFrameFunctions;

int main ()
{
	LogF("piiiico");
}