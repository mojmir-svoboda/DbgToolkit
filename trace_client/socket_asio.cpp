#include "trace.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <trace_proto/header.h>
#include <trace_proto/decoder.h>
#define NOMINMAX 1
#include <asio.hpp>
#include <sysfn/time_query.h>
#include <ScopeGuard.h>

#define DBG_CLIENT 1
#if !defined DBG_CLIENT
#	define DBG_OUT(fmt, ...) ((void)0)
#else
#	include "utils_dbg.h"
#	define DBG_OUT(fmt, ...) dbg_out(fmt, __VA_ARGS__)
#endif

#include "socket_asio_client.h"

namespace trace {

	void OnConnectionEstablished ();
	void OnConnectionConfigCommand (Command const & cmd);

	ClientMemory g_ClientMemory;
	std::unique_ptr<Client> g_Client = nullptr;

	void Connect (char const * host, char const * port)
	{
		DBG_OUT("Trace connecting to server %s:%s\n", host, port);

		sys::setTimeStart();
		SetHostName(host);
		SetHostPort(port);

		g_Client.reset(new Client);
		g_Client->Init(host, port);
	}

	void Disconnect ()
	{
		DBG_OUT("Trace disconnecting...");

		if (g_Client)
		{
			g_Client->Done();
			g_Client.reset();
		}
	}

	void Flush ()
	{
		DBG_OUT("Trace flushing...");

		if (g_Client)
			g_Client->Flush();
	}

	bool WriteToSocket (char const * request, size_t ln)
	{
		if (g_Client)
			return g_Client->WriteToSocket(request, ln);
		return false;
	}
}
