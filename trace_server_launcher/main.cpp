#include <cstdlib>
#include <cstdio>
#include <exception>
#include <string>
#include "test_server.h"
#include "launcher.h"


int main (int argc, char * argv[])
{
#ifdef WIN32
	ShowWindow( GetConsoleWindow(), SW_MINIMIZE);
#endif
	try
	{
		if (argc < 3)
		{
			printf("launcher: ERROR: Insufficient number of arguments\n");

			printf("\tUsage: trace_server_launcher.exe source_exe running_exe args\n");
			printf("\tExample: trace_server_launcher.exe trace_server.exe trace_server.running.exe\n");
			printf("\tExample: trace_server_launcher.exe trace_server.exe trace_server.running.exe -n -q\n");
			return 1;
		}

		std::string args;
		args += argv[2];
		args += " ";
		for (int i = 3; i < argc; ++i)
		{
			args += argv[i];
			args += " ";
		}

		bool const need_update = trace::tryUpdateTraceServer(argv[1], argv[2]);
		if (need_update)
		{
			int count = 0;
			while (isTraceServerRunning())
			{
				tryTraceServerShutdown();
				Sleep(500);
				if (count >= 4)
				{
					printf("launcher: old version not responding. never give up! never surrender!\n");
					Sleep(1000);
				}
				if (count >= 8)
				{
					printf("launcher: old version not responding. d'oh i give up!\n");
					break;
				}
			}

			bool const copy_ok = trace::tryCopyTraceServer(argv[1], argv[2]);
			if (!copy_ok)
			{
				printf("launcher: old version not responding. d'oh, i surrender!\n");
				printf("launcher: trying to start server anyway\n");
			}
			else
			{
				printf("launcher: server updated. starting...\n");
			}
			trace::runTraceServer(argv[2], args.c_str());
		}
		else
		{
			if (!isTraceServerRunning())
			{
				printf("launcher: server already up-to-date, but not running. starting...\n");
				trace::runTraceServer(argv[2], args.c_str());
			}
			else
			{
				printf("launcher: already up-to-date & started. nothing to do.\n");
			}
		}

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

