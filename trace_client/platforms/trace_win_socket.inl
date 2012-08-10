#if defined __MINGW32__
#	undef _WIN32_WINNT
#	define _WIN32_WINNT 0x0600 
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>	// for atoi
#include <cstdio>	// for vsnprintf etc
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#if defined WIN32
#	pragma comment (lib, "Ws2_32.lib")
#	pragma comment (lib, "Mswsock.lib")
#	pragma comment (lib, "AdvApi32.lib")
#elif defined WIN64
#	pragma comment (lib, "Ws2.lib")
#	pragma comment (lib, "Mswsock.lib")
#endif
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_decoder.h>
#include "trace_win_common.inl"
#include "encode_log.inl"
#include "encode_data.inl"
#include "encode_scope.inl"
#include "encode_setup.inl"
#include "encode_exportcsv.inl"

//#define DBG_OUT printf
#define DBG_OUT(fmt, ...) ((void)0)

namespace trace {

	namespace socks {

		sys::atomic32_t volatile g_Quit = 0;			/// request to quit
		CACHE_ALIGN sys::atomic32_t volatile m_wr_idx = 0;		// write index
		MessagePool<msg_t, 1024> g_MessagePool;					// pool of messages
		CACHE_ALIGN sys::atomic32_t volatile m_rd_idx = 0;		// read index

		sys::Thread g_ThreadSend(THREAD_PRIORITY_HIGHEST);		/// consumer-sender thread (high priority)
		sys::Thread g_ThreadRecv(THREAD_PRIORITY_LOWEST);		/// receiving thread (low priority)
		typedef SOCKET socket_t;
		socket_t g_Socket = INVALID_SOCKET;
		HANDLE g_LogFile = INVALID_HANDLE_VALUE;
		sys::Timer g_ReconnectTimer;
		sys::Timer g_ClottedTimer;

		inline msg_t & msg_buffer_at (size_t i)
		{
			return g_MessagePool[i];
		}

		inline msg_t & acquire_msg_buffer ()
		{
			sys::atomic32_t wr_idx = InterlockedIncrement(&m_wr_idx);
			return msg_buffer_at((wr_idx - 1) % MessagePool<msg_t, 1024>::e_size);
		}

		inline bool is_connected () { return g_Socket != INVALID_SOCKET; }
		inline bool is_file_connected () { return g_LogFile != INVALID_HANDLE_VALUE; }

		inline bool WriteToFile (char const * buff, size_t ln)
		{
			if (is_file_connected())
			{
				DWORD written;
				WriteFile(g_LogFile, buff, static_cast<DWORD>(ln), &written, 0);
				return true;
			}
			return false;
		}

		int get_errno () { return WSAGetLastError(); }
		bool is_timeouted () { return get_errno() == WSAETIMEDOUT; }

		inline bool WriteToSocket (char const * buff, size_t ln)
		{
			if (is_connected())
			{
				int result = SOCKET_ERROR;
				if (!g_ClottedTimer.enabled())
					result = send(g_Socket, buff, (int)ln, 0);
				else
				{
					if (g_ClottedTimer.expired())
					{
						result = send(g_Socket, buff, (int)ln, 0);
						if (result != SOCKET_ERROR && result > 0)
						{
							DBG_OUT("declotted\n");
							g_ClottedTimer.reset();
						}
					}
					else
					{
#ifdef TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
						WriteToFile(buff, ln);
#endif
						return true;
					}
				}

				bool const timeouted = ( result == SOCKET_ERROR && is_timeouted());
				if (result == SOCKET_ERROR && !timeouted)
				{
					DBG_OUT("send failed with error: %d\n", get_errno());
					closesocket(g_Socket);
					WSACleanup();
					g_Socket = INVALID_SOCKET;

#ifdef TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
					WriteToFile(buff, ln);
#endif
					return false;
				}
				else if (timeouted || result == 0)
				{
					DBG_OUT("socked clotted\n");
					g_ClottedTimer.set_delay_ms(128);
					return true;
				}
			}
#ifdef TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
			else
				WriteToFile(buff, ln);
#endif
			DBG_OUT(".");
			return true;
		}

		/**@brief	function receiving commands from server **/
		DWORD WINAPI receive_thread ( LPVOID )
		{
			enum { e_buff_sz = 256 };
			char buff[e_buff_sz];
			tlv::Command<256, 8> cmd; // reserve 4 tlvs with maximum of 128 bytes of concatenated data
			while (!g_Quit)
			{
				int const result = recv(g_Socket, buff, e_buff_sz, 0); //@TODO: better recv logic (this is sufficent for now)
				if (result > 0)
				{
					tlv::TLVDecoder d;
					if (d.decode_header(buff, result, cmd))
					{
						if (d.decode_payload(buff + tlv::Header::e_Size, cmd.hdr.len, cmd))
						{
							if (cmd.hdr.cmd == tlv::cmd_set_level && cmd.tlvs_count > 0)
							{
								printf("level changed!\n");
								SetRuntimeLevel(static_cast<trace::level_t>(atoi(cmd.tlvs[0].m_val)));
							}

							if (cmd.hdr.cmd == tlv::cmd_set_buffering && cmd.tlvs_count > 0)
							{
								unsigned const buff_state = atoi(cmd.tlvs[0].m_val);
								char grr[256];
								_snprintf_s(grr, 256, "buffering changed! val=%u\n", buff_state);
								OutputDebugString(grr);
								SetRuntimeBuffering(buff_state == 1);
							}
						}
					}
				}
				else if (result == 0)
				{
					DBG_OUT("Connection closed\n");
					break;
				}
				else
				{
					DBG_OUT("recv failed: %d\n", get_errno());
					break;
				}
			}
			return 0;
		}

		bool try_connect ();

		/**@brief	function consuming and sending items from MessagePool **/
		DWORD WINAPI consumer_thread ( LPVOID )
		{
			while (!g_Quit)
			{
				if (g_ReconnectTimer.enabled() && g_ReconnectTimer.expired())
				{
					if (try_connect())
						g_ReconnectTimer.reset();
					else
						g_ReconnectTimer.set_delay_ms(1000);
				}
				sys::atomic32_t wr_idx = sys::atomic_get32(&m_wr_idx);
				sys::atomic32_t rd_idx = m_rd_idx;
				// @TODO: wraparound
				if (rd_idx < wr_idx)
				{
					//DBG_OUT("rd_idx=%10i, wr_idx=%10i, diff=%10i \n", rd_idx, wr_idx, wr_idx - rd_idx);
					msg_t & msg = socks::msg_buffer_at(rd_idx % MessagePool<msg_t, 1024>::e_size);
					msg.ReadLock();

					bool const write_ok = socks::WriteToSocket(msg.m_data, msg.m_length);
					msg.m_length = 0;
					msg.ReadUnlockAndClean();
					++m_rd_idx;

					if (!write_ok)
					{
						g_ReconnectTimer.set_delay_ms(1000);
						//break;
					}
				}
				else
					sys::thread_yield();
			}
			return 0;
		}
	}

	namespace socks {
		void connect (char const * host, char const * port, socket_t & handle)
		{
			WSADATA wsaData;
			int const init_result = WSAStartup(MAKEWORD(2,2), &wsaData);
			if (init_result != 0) {
				DBG_OUT("WSAStartup failed with error: %d\n", init_result);
				return;
			}

			addrinfo hints;
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			addrinfo * result = NULL;
			int const resolve_result = getaddrinfo(host, port, &hints, &result); // resolve the server address and port
			if (resolve_result != 0) {
				DBG_OUT("getaddrinfo failed with error: %d\n", resolve_result);
				WSACleanup();
				return;
			}

			// attempt to connect to an address until one succeeded
			for (addrinfo * ptr = result; ptr != NULL ; ptr = ptr->ai_next) {
				// create a SOCKET for connecting to server
				handle = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if (handle == INVALID_SOCKET) {
					DBG_OUT("socket failed with error: %ld\n", get_errno());
					WSACleanup();
					return;
				}

				// Connect to server.
				int const connect_result = connect(handle, ptr->ai_addr, (int)ptr->ai_addrlen);
				if (connect_result == SOCKET_ERROR) {
					closesocket(handle);
					handle = INVALID_SOCKET;
					continue;
				}
				break;
			}

			freeaddrinfo(result);

			if (handle == INVALID_SOCKET)
			{
				DBG_OUT("Unable to connect to server!\n");
				WSACleanup();
				return;
			}

			int send_buff_sz = 64 * 1024;
			setsockopt(handle, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char *>(&send_buff_sz), sizeof(int));

			DWORD send_timeout_ms = 1;
			setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&send_timeout_ms), sizeof(DWORD));
		}

		bool try_connect ()
		{
			g_ReconnectTimer.reset();
			g_ClottedTimer.reset();
			msg_t msg;
			// send cmd_setup message
			encode_setup(msg, GetRuntimeLevel(), GetRuntimeContextMask());

			socks::connect("localhost", "13127", g_Socket);

			if (socks::is_connected())
			{
				socks::WriteToSocket(msg.m_data, msg.m_length);

				socks::g_ThreadRecv.Create(socks::receive_thread, 0);
				socks::g_ThreadRecv.Resume();
				return true;
			}
			return false;
		}
	}

	void Connect ()
	{
		sys::setTimeStart();

		socks::g_ThreadSend.Create(socks::consumer_thread, 0);

		bool const connected = socks::try_connect();
		if (!connected)
			socks::g_ReconnectTimer.set_delay_ms(1000);

#	if defined TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
		char filename[128];
		create_log_filename(filename, sizeof(filename) / sizeof(*filename));
		socks::g_LogFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		msg_t msg;
		// send cmd_setup message
		encode_setup(msg, GetRuntimeLevel(), GetRuntimeContextMask());
		socks::WriteToFile(msg.m_data, msg.m_length);
#	endif

		socks::g_ThreadSend.Resume();
	}

	void Disconnect ()
	{
		socks::g_Quit = 1; // @TODO: atomic_store?
		if (socks::is_connected())
		{
			int const result = shutdown(socks::g_Socket, SD_SEND);
			if (result == SOCKET_ERROR)
				DBG_OUT("shutdown failed with error: %d\n", get_errno());
			closesocket(socks::g_Socket);
			socks::g_Socket = INVALID_SOCKET;
			WSACleanup();
		}
		socks::g_ThreadSend.WaitForTerminate();
		socks::g_ThreadSend.Close();
		socks::g_ThreadRecv.WaitForTerminate();
		socks::g_ThreadRecv.Close();

#	if defined TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
		CloseHandle(socks::g_LogFile);
#	endif
	}

	inline void WriteLog (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		if (GetRuntimeBuffering())
		{
			msg_t & msg = socks::acquire_msg_buffer();
			msg.WriteLock();
			{
				encode_log(msg, level, context, file, line, fn, fmt, args);
			}
			msg.WriteUnlockAndDirty();
		}
		else
		{
			msg_t msg;
			encode_log(msg, level, context, file, line, fn, fmt, args);
			socks::WriteToSocket(msg.m_data, msg.m_length);
		}
	}

	inline void WriteStr (level_t level, context_t context, char const * file, int line, char const * fn, char const * str)
	{
		if (GetRuntimeBuffering())
		{
			msg_t & msg = socks::acquire_msg_buffer();
			msg.WriteLock();
			{
				encode_str(msg, level, context, file, line, fn, str);
			}
			msg.WriteUnlockAndDirty();
		}
		else
		{
			msg_t msg;
			encode_str(msg, level, context, file, line, fn, str);
			socks::WriteToSocket(msg.m_data, msg.m_length);
		}
	}

	inline void WriteScope (ScopedLog::E_Type type, level_t level, context_t context, char const * file, int line, char const * fn)
	{
		if (GetRuntimeBuffering())
		{
			msg_t & msg = socks::acquire_msg_buffer();
			msg.WriteLock();
			{
				encode_scope(msg, type == ScopedLog::e_Entry ? tlv::cmd_scope_entry : tlv::cmd_scope_exit , level, context, file, line, fn);
			}
			msg.WriteUnlockAndDirty();
		}
		else
		{
			msg_t msg;
			encode_scope(msg, type == ScopedLog::e_Entry ? tlv::cmd_scope_entry : tlv::cmd_scope_exit , level, context, file, line, fn);
			socks::WriteToSocket(msg.m_data, msg.m_length);
		}
	}

	inline void ExportToCSV (char const * file)
	{
		if (GetRuntimeBuffering())
		{
			msg_t & msg = socks::acquire_msg_buffer();
			msg.WriteLock();
			{
				encode_exportCSV(msg, file);
			}
			msg.WriteUnlockAndDirty();
		}
		else
		{
			msg_t msg;
			encode_exportCSV(msg, file);
			socks::WriteToSocket(msg.m_data, msg.m_length);
		}

	}

	inline void WriteData_impl (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
	{
		if (GetRuntimeBuffering())
		{
			msg_t & msg = socks::acquire_msg_buffer();
			msg.WriteLock();
			{
				encode_data(msg, level, context, x, y, fmt, args);
			}
			msg.WriteUnlockAndDirty();
		}
	}

	inline void WriteDataXYZ (level_t level, context_t context, float x, float y, float z, char const * fmt, va_list args)
	{
		if (GetRuntimeBuffering())
		{
			msg_t & msg = socks::acquire_msg_buffer();
			msg.WriteLock();
			{
				encode_data(msg, level, context, x, y, z, fmt, args);
			}
			msg.WriteUnlockAndDirty();
		}
	}
}

