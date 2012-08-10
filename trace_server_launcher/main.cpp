#include <cstdlib>
#include <cstdio>
#include "test_server.h"
#include "launcher.h"


int main (int argc, char * argv[])
{
	try
	{
		char const * log = 0;
		if (argc < 3)
		{
			printf("launcher: ERROR: Wrong number of arguments\n");

			printf("\tUsage: trace_server_launcher.exe source_exe running_exe\n");
			printf("\tExample: trace_server_launcher.exe trace_server.exe trace_server_running.exe\n");
			return 1;
		}

		if (argc == 3)
			log = argv[2];

		printf("launcher: starting test server...\n");

		trace::tryUpdateTraceServer(argv[1], argv[2]);
		trace::runTraceServer(argv[1], argv[2]);
	}
	catch (std::exception & e)
	{
		printf("launcher: ERROR - Exception in thread: %s\n", e.what());
		fflush(stdout);
	}
	catch (...)
	{
		printf("launcher: ERROR - General Failure!\n");
		return 10;
	}
	return 0;
}

