#include <cstdio>
#include "launcher.h"

int main (int argc, char * argv[])
{
	if (argc < 3)
	{
		printf("Error: Wrong number of arguments\n");
		return 1;
	}

	printf("TraceServer launcher...\n");

	char const * g_TraceName = "trace_server.exe";
	char const * g_TraceNameRun = "trace_server_running.exe";
	try {

		trace::runTraceServer(g_TraceName, g_TraceNameRun);
	}
	catch (...)
	{
		printf("Error: General Failure!\n");
		return 10;
	}
	return 0;
}
