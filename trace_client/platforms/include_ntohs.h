#pragma once
#if defined WIN32 || defined WIN64
#	include <ws2tcpip.h>
#elif defined __linux__
#	include <arpa/inet.h>
#endif
