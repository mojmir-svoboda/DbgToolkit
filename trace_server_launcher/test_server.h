#define TRACE_ENABLE
#define TRACE_WINDOWS_USES_SOCKET
//#include <trace_client/default_config.h>
//#include <trace_client/trace.h>
#include <sysfn/socket_win.h>
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>
#include <sysfn/os.h>

bool writeToSocket (sys::socks::socket_t & socket, char const * buff, size_t ln)
{
	if (sys::socks::is_connected(socket))
	{
		int const result = send(socket, buff, (int)ln, 0);
		if (result != SOCKET_ERROR && result > 0)
			return true;
	}
	return false;
}

bool isTraceServerRunning (char const * port = "13127")
{
	sys::socks::socket_t socket = INVALID_SOCKET;
	sys::socks::connect("localhost", port, socket);
	bool const is_running = socket != INVALID_SOCKET;
	if (is_running)
	{
		char data[64];
		tlv::Encoder e(tlv::cmd_ping, data, 64);
		size_t const tlv_buff_sz = 64;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag_app,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%s", "trace_server_launcher"), tlv_buff));
		if (e.Commit())
		{
			if (writeToSocket(socket, data, e.total_len))
				return true;
		}

		closesocket(socket);
		socket = INVALID_SOCKET;
	}
	return is_running;
}

bool tryTraceServerShutdown (char const * port = "13127")
{
	sys::socks::socket_t socket = INVALID_SOCKET;
	sys::socks::connect("localhost", port, socket);
	bool is_running = socket != INVALID_SOCKET;
	if (is_running)
	{
		char data[64];
		tlv::Encoder e(tlv::cmd_shutdown, data, 64);
		size_t const tlv_buff_sz = 64;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag_app,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%s", "trace_server_launcher"), tlv_buff));
		if (e.Commit())
		{
			is_running = writeToSocket(socket, data, e.total_len);
		}

		closesocket(socket);
		socket = INVALID_SOCKET;
	}
	return is_running;

}
