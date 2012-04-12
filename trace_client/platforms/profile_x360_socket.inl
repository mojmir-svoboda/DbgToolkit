#pragma once
#include <cstdlib>	// for atoi
#include <cstdio>	// for vsnprintf etc
#include "profile_x360_common.inl"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_decoder.h>
#include "profile_encode_bgn.inl"
#include "profile_encode_end.inl"
#include "profile_encode_setup.inl"

//#define DBG_OUT printf
#define DBG_OUT(fmt, ...) ((void)0)

namespace profile {

	namespace socks {

		sys::atomic32_t volatile g_Quit = 0;			/// request to quit
		CACHE_ALIGN sys::atomic32_t volatile m_wr_idx = 0;		// write index

		typedef MessagePool<msg_t, 1024> pool_t;
		pool_t g_MessagePool;			                        /// pool of messages
		CACHE_ALIGN sys::atomic32_t volatile m_rd_idx = 0;		// read index

		sys::Thread g_ThreadSend(THREAD_PRIORITY_HIGHEST);		/// consumer-sender thread (high priority)
		SOCKET g_Socket = INVALID_SOCKET;
		sys::Timer g_ReconnectTimer;

		inline msg_t & msg_buffer_at (size_t i) { return g_MessagePool[i]; }
		inline msg_t & acquire_msg_buffer ()
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
				if (SOCKET_ERROR == send(g_Socket, buff, ln, 0))
				{
					DBG_OUT("send failed with error: %d\n", get_errno());
					closesocket(g_Socket);
					g_Socket = INVALID_SOCKET;
					return true;
				}
				return false;
			}
			return true;
		}

		bool try_connect ();

		/**@brief	function consuming and sending items from MessagePool **/
		DWORD WINAPI consumer_thread ( LPVOID )
		{
			while (!g_Quit)
			{
				sys::atomic32_t wr_idx = sys::atomic_get32(&m_wr_idx);
				sys::atomic32_t rd_idx = m_rd_idx;
				// @TODO: wraparound
				if (rd_idx < wr_idx)
				{
					//DBG_OUT("rd_idx=%10i, wr_idx=%10i, diff=%10i \n", rd_idx, wr_idx, wr_idx - rd_idx);
					msg_t & msg = socks::msg_buffer_at(rd_idx % pool_t::e_size);
					msg.ReadLock();

					socks::WriteToSocket(msg.m_data, msg.m_length);
					msg.m_length = 0;
					msg.ReadUnlockAndClean();
					++m_rd_idx;
				}
				else
					sys::thread_yield();
			}

			// flush remaining messages

			sys::atomic32_t wr_idx = sys::atomic_get32(&m_wr_idx);
			sys::atomic32_t rd_idx = m_rd_idx;
			while (rd_idx < wr_idx)
			{
				//DBG_OUT("rd_idx=%10i, wr_idx=%10i, diff=%10i \n", rd_idx, wr_idx, wr_idx - rd_idx);
				msg_t & msg = socks::msg_buffer_at(rd_idx % pool_t::e_size);
				msg.ReadLock();

				socks::WriteToSocket(msg.m_data, msg.m_length);
				msg.m_length = 0;
				msg.ReadUnlockAndClean();
				++m_rd_idx;
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

			g_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if ((g_Socket == SOCKET_ERROR) || (g_Socket == INVALID_SOCKET))
			{
				printf("Trace ERR: socket error %ld\n", WSAGetLastError());
			}

			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = inet_addr(host);
			//if (addr.sin_addr.s_addr == INADDR_NONE)
			//	return NO_ERROR == connectViaDNS(g_Socket, host, port);

			if (connect(g_Socket, (struct sockaddr*) &addr, sizeof(addr)) != 0)
			{
				printf("Trace ERR: connect error: %ld\n", WSAGetLastError());
				g_Socket = INVALID_SOCKET;
				return;
			}
			printf("Trace OK: connected to TraceServer\n");

			int send_buff_sz = 64 * 1024;
			setsockopt(g_Socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char *>(&send_buff_sz), sizeof(int));
			DWORD send_timeout_ms = 1;
			setsockopt(g_Socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&send_timeout_ms), sizeof(DWORD));
		}

		bool try_connect ()
		{
			g_ReconnectTimer.reset();
			msg_t msg;
			// send cmd_setup message
			encode_setup(msg);

			socks::connect("192.168.1.81", "13127");
			//socks::connect("localhost", "13147");
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
		sys::setTimeStart();
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
			shutdown(socks::g_Socket, SD_BOTH);
			closesocket(socks::g_Socket);
			__lwsync();

			socks::g_Socket = INVALID_SOCKET;
		}
		socks::g_ThreadSend.WaitForTerminate();
		socks::g_ThreadSend.Close();
	}


	inline void WriteBgn_Impl ()
	{
		msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_bgn(msg);
		}
		msg.WriteUnlockAndDirty();
	}
	inline void WriteBgnVA (char const * fmt, va_list args)
	{
		msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_bgn(msg, fmt, args);
		}
		msg.WriteUnlockAndDirty();
	}

	inline void WriteEnd_Impl ()
	{
		msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_end(msg);
		}
		msg.WriteUnlockAndDirty();
	}
	inline void WriteEndVA (char const * fmt, va_list args)
	{
		msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_end(msg, fmt, args);
		}
		msg.WriteUnlockAndDirty();
	}

	inline void WriteFrameBgn_Impl ()
	{
		msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_frame_bgn(msg);
		}
		msg.WriteUnlockAndDirty();
	}
	inline void WriteFrameBgnVA (char const * fmt, va_list args)
	{
		msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_frame_bgn(msg, fmt, args);
		}
		msg.WriteUnlockAndDirty();
	}

	inline void WriteFrameEnd_Impl ()
	{
		msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_frame_end(msg);
		}
		msg.WriteUnlockAndDirty();
	}
	inline void WriteFrameEndVA (char const * fmt, va_list args)
	{
		msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_frame_end(msg, fmt, args);
		}
		msg.WriteUnlockAndDirty();
	}

}

