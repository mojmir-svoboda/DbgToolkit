#define TRACE_ENABLE
#include <trace_client/trace.h>
//#include <trace_client/platforms/select_platform.inl>


bool isTraceServerRunning (char const * port = "13127")
{

	trace::socks::socket_t socket = INVALID_SOCKET;
	trace::socks::connect("localhost", port, socket);
	bool const is_running = socket != INVALID_SOCKET;
	if (is_running)
	{

		closesocket(socket);
		socket = INVALID_SOCKET;
	}
	return is_running;
}
