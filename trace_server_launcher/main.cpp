#include <cstdlib>
#include <cstdio>
#include <exception>
#include <string>
#include "test_server.h"
#include "launcher.h"

enum {
	e_Launch_OK,
	e_Launch_NoSrc,
	e_Launch_IO_Exception,
	e_Launch_General_Failure
};

int main (int argc, char * argv[])
{
	freopen("trace_server_launcher.log", "w", stdout);

#ifdef WIN32
	ShowWindow( GetConsoleWindow(), SW_MINIMIZE);
	setvbuf(stdout, 0, _IONBF, 0); // unbuffered output
#endif
	try
	{
		char const * src_exe = 0;
		char const * run_exe = 0;
		if (argc < 3)
		{
#ifdef WIN32
			src_exe = "trace_server.exe";
			run_exe = "trace_server.run.exe";
#else
			src_exe = "trace_server";
			run_exe = "trace_server.run";
#endif
			printf("launcher: Insufficient number of arguments, using defaults:\n");
			printf("\ttrace_server_launcher.exe %s %s\n", src_exe, run_exe);

			printf("Usage:\n");
			printf("\tUsage: trace_server_launcher.exe source_exe running_exe args\n");
			printf("\tExample: trace_server_launcher.exe trace_server.exe trace_server.running.exe\n");
			printf("\tExample: trace_server_launcher.exe trace_server.exe trace_server.running.exe -n -q\n");
		}
		else
		{
			src_exe = argv[1];
			run_exe = argv[2];
		}

		std::string args;
		args += run_exe;
		args += " ";
		for (int i = 3; i < argc; ++i)
		{
			args += argv[i];
			args += " ";
		}

		if (!trace::fileExists(src_exe))
		{
			printf("launcher: Error: source file not present, nothing to do\n");
			return e_Launch_NoSrc;
		}

		if (!trace::fileExists(run_exe))
		{
			printf("launcher: waiting for any running server to shut down...\n");
			trace::tryShutdownRunningServer(2);

			printf("launcher: target run file not existing, copying...\n");
			trace::tryCopyTraceServer(src_exe, run_exe);
		}
		else
		{
			bool const need_update = trace::tryUpdateTraceServer(src_exe, run_exe);
			if (need_update)
			{
				printf("launcher: waiting for running server to shut down...\n");
				trace::tryShutdownRunningServer(10);

				bool const copy_ok = trace::tryCopyTraceServer(src_exe, run_exe);
				if (!copy_ok)
				{
					printf("launcher: old version not responding. d'oh, i surrender.. DIY pls!\n");
				}
				else
				{
					printf("launcher: server updated. starting...\n");
				}
			}
		}

		if (!isTraceServerRunning())
		{
			printf("launcher: server already up-to-date, but not running. starting...\n");
			trace::runTraceServer(run_exe, args.c_str());
		}
		else
		{
			printf("launcher: already up-to-date & started.\n");
		}
	}
	catch (std::exception & e)
	{
		printf("launcher: ERROR - Exception in IO thread: %s\n", e.what());
		fflush(stdout);
		return e_Launch_IO_Exception;
	}
	catch (...)
	{
		printf("launcher: ERROR - General Failure!\n");
		return e_Launch_General_Failure;
	}

#ifdef WIN32
	//system("pause");
#endif
	return e_Launch_OK;
}

