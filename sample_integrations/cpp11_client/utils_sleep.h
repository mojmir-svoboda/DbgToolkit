#pragma once

void sleep_ms (int ms)
{
#if defined WIN32 || defined WIN64
	Sleep(ms);
#elif defined __linux__
	usleep(ms * 1000);
#endif
}


