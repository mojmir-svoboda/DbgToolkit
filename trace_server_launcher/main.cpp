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

	try {
		trace::tryUpdateTraceServer(argv[1], argv[2]);
		trace::runTraceServer(argv[1], argv[2]);
	}
	catch (...)
	{
		printf("Error: General Failure!\n");
		return 10;
	}
	return 0;
}
