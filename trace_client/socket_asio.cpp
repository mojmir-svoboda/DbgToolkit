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

	bool Init (char const * appName)
	{
		g_Client.reset(new Client);
		if (g_Client)
		{
			g_Client->m_config.SetAppName(appName);
		}
		return g_Client.get() != nullptr;
	}

	void Done ()
	{
		g_Client.reset();
	}

	void SetRuntimeLevelForContext (context_t ctx, level_t level)
	{
		g_Client->m_config.SetRuntimeLevelForContext(ctx, level);
	}
	level_t GetRuntimeLevelForContextBit (context_t b)
	{
		return g_Client->m_config.GetRuntimeLevelForContextBit(b);
	}
	//level_t * GetRuntimeCfgData () { return g_Config.m_mixer.data(); }
	//size_t GetRuntimeCfgSize () { return g_Config.m_mixer.size(); }

	void SetRuntimeBuffering (bool buffered) { g_Client->m_config.m_buffered = buffered; }
	bool GetRuntimeBuffering () { return g_Client->m_config.m_buffered; }
	void SetLevelDictionary (level_t const * values, char const * names[], size_t sz)
	{
		g_Client->m_config.SetLevelDictionary(values, names, sz);
	}
	void SetContextDictionary (context_t const * values, char const * names[], size_t sz)
	{
		g_Client->m_config.SetContextDictionary(values, names, sz);
	}
	
	void Connect (char const * host, char const * port)
	{
		DBG_OUT("Trace connecting to server %s:%s\n", host, port);

		sys::setTimeStart();
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
