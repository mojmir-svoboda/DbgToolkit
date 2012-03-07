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
#include "profile_win_common.inl"
#include "profile_encode_bgn.inl"
#include "profile_encode_end.inl"
#include "profile_encode_setup.inl"

//#define DBG_OUT printf
#define DBG_OUT(fmt, ...) ((void)0)

namespace profile {

	namespace socks {

		sys::atomic32_t volatile g_Quit = 0;			/// request to quit
		CACHE_ALIGN sys::atomic32_t volatile m_wr_idx = 0;		// write index

		typedef sys::MessagePool<sys::msg_t, 1024> pool_t;
		pool_t g_MessagePool;			                        /// pool of messages
		CACHE_ALIGN sys::atomic32_t volatile m_rd_idx = 0;		// read index

		sys::Thread g_ThreadSend(THREAD_PRIORITY_HIGHEST);		/// consumer-sender thread (high priority)
		SOCKET g_Socket = INVALID_SOCKET;
		sys::Timer g_ReconnectTimer;

		inline sys::msg_t & msg_buffer_at (size_t i) { return g_MessagePool[i]; }
		inline sys::msg_t & acquire_msg_buffer ()
		{
			sys::atomic32_t wr_idx = InterlockedIncrement(&m_wr_idx);
			return msg_buffer_at((wr_idx - 1) % pool_t::e_size);
		}

		inline bool is_connected () { return g_Socket != INVALID_SOCKET; }

		int get_errno () { return WSAGetLastError(); }
		bool is_timeouted () { return get_errno() == WSAETIMEDOUT; }

		inline bool WriteToSocket (char const * buff, size_t ln)
		{
			if (is_connected())
			{
				int const result = send(g_Socket, buff, (int)ln, 0);
				if (result == SOCKET_ERROR)
				{
					DBG_OUT("send failed with error: %d\n", get_errno());
					closesocket(g_Socket);
					WSACleanup();
					g_Socket = -1;
					return false;
				}
			}
			return true;
		}

		bool try_connect ();

		/**@brief	function consuming and sending items from MessagePool **/
		DWORD WINAPI consumer_thread ( LPVOID )
		{
			while (!g_Quit)
			{
				sys::atomic32_t wr_idx = sys::atomic_get(&m_wr_idx);
				sys::atomic32_t rd_idx = m_rd_idx;
				// @TODO: wraparound
				if (rd_idx < wr_idx)
				{
					//DBG_OUT("rd_idx=%10i, wr_idx=%10i, diff=%10i \n", rd_idx, wr_idx, wr_idx - rd_idx);
					sys::msg_t & msg = socks::msg_buffer_at(rd_idx % pool_t::e_size);
					msg.ReadLock();

					socks::WriteToSocket(msg.m_data, msg.m_length);
					msg.m_length = 0;
					msg.ReadUnlockAndClean();
					++m_rd_idx;
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
				g_Socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if (g_Socket == INVALID_SOCKET) {
					DBG_OUT("socket failed with error: %ld\n", get_errno());
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
				DBG_OUT("Unable to connect to server!\n");
				WSACleanup();
				return;
			}

			int send_buff_sz = 64 * 1024;
			setsockopt(g_Socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char *>(&send_buff_sz), sizeof(int));

			DWORD send_timeout_ms = 1;
			setsockopt(g_Socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&send_timeout_ms), sizeof(DWORD));
		}

		bool try_connect ()
		{
			g_ReconnectTimer.reset();
			sys::msg_t msg;
			// send cmd_setup message
			encode_setup(msg);

			socks::connect("localhost", "13147");
			if (socks::is_connected())
			{
				socks::WriteToSocket(msg.m_data, msg.m_length);
				return true;
			}
			return false;
		}
	}

	void Connect ()
	{
		sys::SetTickStart();
		socks::g_ThreadSend.Create(socks::consumer_thread, 0);

		bool const connected = socks::try_connect();
		if (!connected)
			socks::g_ReconnectTimer.set_delay_ms(1000);

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
	}

	inline void WriteBgnVA (char const * fmt, va_list args)
	{
		sys::msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_bgn(msg, fmt, args);
		}
		msg.WriteUnlockAndDirty();
	}

	inline void WriteEnd ()
	{
		sys::msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_end(msg);
		}
		msg.WriteUnlockAndDirty();
	}

	inline void WriteFrameBgnVA (char const * fmt, va_list args)
	{
		sys::msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_frame_bgn(msg, fmt, args);
		}
		msg.WriteUnlockAndDirty();
	}

	inline void WriteFrameEnd ()
	{
		sys::msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_frame_end(msg);
		}
		msg.WriteUnlockAndDirty();
	}

}

