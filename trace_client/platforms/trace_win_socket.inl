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
#include "../../tlv_parser/tlv_parser.h"
#include "../../tlv_parser/tlv_decoder.h"
#include "trace_win_common.inl"
#include "encode_log.inl"
#include "encode_scope.inl"
#include "encode_setup.inl"

namespace trace {

	namespace socks {

		LONG volatile g_Quit = 0;			/// request to quit
		CACHE_ALIGN LONG volatile m_wr_idx = 0;		// write index
		sys::MessagePool g_MessagePool;					// pool of messages
		CACHE_ALIGN LONG volatile m_rd_idx = 0;		// read index

		sys::Thread g_ThreadSend(THREAD_PRIORITY_HIGHEST);		/// consumer-sender thread (high priority)
		sys::Thread g_ThreadRecv(THREAD_PRIORITY_LOWEST);		/// receiving thread (low priority)
		SOCKET g_Socket = INVALID_SOCKET;
		HANDLE g_LogFile = INVALID_HANDLE_VALUE;

		inline sys::Message & msg_buffer_at (size_t i)
		{
			return g_MessagePool[i];
		}

		inline sys::Message & acquire_msg_buffer ()
		{
			LONG wr_idx = InterlockedIncrement(&m_wr_idx);
			return msg_buffer_at((wr_idx - 1) % sys::MessagePool::e_size);
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

		inline bool WriteToSocket (char const * buff, size_t ln)
		{
			if (is_connected())
			{
				int const result = send(g_Socket, buff, (int)ln, 0);
				if (result == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(g_Socket);
					WSACleanup();
					g_Socket = INVALID_SOCKET;

					WriteToFile(buff, ln);
					//return false;
				}
			}
#ifdef TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
			else
				WriteToFile(buff, ln);
#endif
			printf(".");
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
								SetRuntimeLevel(static_cast<trace::level_t>(atoi(cmd.tlvs[0].m_val)));
							}
						}
					}
				}
				else if (result == 0)
				{
					printf("Connection closed\n");
					break;
				}
				else
				{
					printf("recv failed: %d\n", WSAGetLastError());
					break;
				}
			}
			return 0;
		}

		/**@brief	function consuming and sending items from MessagePool **/
		DWORD WINAPI consumer_thread ( LPVOID )
		{
			while (!g_Quit)
			{
				LONG wr_idx = sys::atomic_get(&m_wr_idx);
				LONG rd_idx = m_rd_idx;
				// @TODO: wraparound
				if (rd_idx < wr_idx)
				{
					//printf("rd_idx=%10i, wr_idx=%10i, diff=%10i \n", rd_idx, wr_idx, wr_idx - rd_idx);
					sys::Message & msg = socks::msg_buffer_at(rd_idx % sys::MessagePool::e_size);
					msg.ReadLock();

					bool const write_ok = socks::WriteToSocket(msg.m_data, msg.m_length);
					msg.m_length = 0;
					msg.ReadUnlockAndClean();
					++m_rd_idx;

					if (!write_ok)
						break;
				}
				else
					sys::thread_yield();
			}
			return 0;
		}
	}

	namespace socks {
		void connect (char const * host, char const * port)
		{
			WSADATA wsaData;
			int const init_result = WSAStartup(MAKEWORD(2,2), &wsaData);
			if (init_result != 0) {
				printf("WSAStartup failed with error: %d\n", init_result);
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
				printf("getaddrinfo failed with error: %d\n", resolve_result);
				WSACleanup();
				return;
			}

			// attempt to connect to an address until one succeeded
			for (addrinfo * ptr = result; ptr != NULL ; ptr = ptr->ai_next) {
				// create a SOCKET for connecting to server
				g_Socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if (g_Socket == INVALID_SOCKET) {
					printf("socket failed with error: %ld\n", WSAGetLastError());
					WSACleanup();
					return;
				}

				// Connect to server.
				int const connect_result = connect(g_Socket, ptr->ai_addr, (int)ptr->ai_addrlen);
				if (connect_result == SOCKET_ERROR) {
					closesocket(g_Socket);
					g_Socket = INVALID_SOCKET;
					continue;
				}
				break;
			}

			freeaddrinfo(result);

			if (g_Socket == INVALID_SOCKET)
			{
				printf("Unable to connect to server!\n");
				WSACleanup();
				return;
			}

			int send_buff_sz = 64 * 1024;
			setsockopt(g_Socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char *>(&send_buff_sz), sizeof(int));
		}
	}

	void Connect ()
	{
		sys::Message msg;
		// send cmd_setup message
		encode_setup(msg, GetRuntimeLevel(), GetRuntimeContextMask());

		socks::connect("localhost", "13127");

		socks::g_ThreadSend.Create(socks::consumer_thread, 0);
		sys::SetTickStart();

		if (socks::is_connected())
		{
			socks::WriteToSocket(msg.m_data, msg.m_length);

			socks::g_ThreadRecv.Create(socks::receive_thread, 0);
			socks::g_ThreadRecv.Resume();
		}
#	if defined TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
		char filename[128];
		sys::create_log_filename(filename, sizeof(filename) / sizeof(*filename));
		socks::g_LogFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
				printf("shutdown failed with error: %d\n", WSAGetLastError());
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
		sys::Message & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_log(msg, level, context, file, line, fn, fmt, args);
		}
		msg.WriteUnlockAndDirty();
	}

	inline void WriteStr (level_t level, context_t context, char const * file, int line, char const * fn, char const * str)
	{
		sys::Message & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_str(msg, level, context, file, line, fn, str);
		}
		msg.WriteUnlockAndDirty();
	}

	inline void WriteScope (ScopedLog::E_Type type, level_t level, context_t context, char const * file, int line, char const * fn)
	{
		sys::Message & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_scope(msg, type == ScopedLog::e_Entry ? tlv::cmd_scope_entry : tlv::cmd_scope_exit , level, context, file, line, fn);
		}
		msg.WriteUnlockAndDirty();
	}
}

