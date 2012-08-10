#include <cstdlib>
#include <cstdio>
#include <exception>
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

		printf("launcher: is server running?\n");

		bool const need_update = trace::tryUpdateTraceServer(argv[1], argv[2]);
		if (need_update)
		{
			int count = 0;
			while (isTraceServerRunning())
			{
				printf("launcher: shutting down server...\n");
				tryTraceServerShutdown();
				Sleep(500);
				if (count >= 4)
				{
					printf("launcher: old version not responding. never give up! never surrender!\n");
					Sleep(1000);
				}
				if (count >= 8)
				{
					printf("launcher: old version not responding. oh i give up...\n");
					break;
				}
			}

			bool const copy_ok = trace::tryCopyTraceServer(argv[1], argv[2]);
			if (!copy_ok)
				printf("launcher: old version not responding. oh and i surrender too!\n");
		}

		printf("launcher: starting server...\n");
		trace::runTraceServer(argv[2], "-n -q"); //@TODO: from argv
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
	//system("pause");
	return 0;
}

