#pragma once
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_decoder.h>
#include "profile_linux_common.inl"
#include "profile_encode_bgn.inl"
#include "profile_encode_end.inl"
#include "profile_encode_setup.inl"

//#define DBG_OUT printf
#define DBG_OUT(fmt, ...) ((void)0)

namespace profile {

	namespace socks {

		sys::atomic32_t volatile g_Quit = 0;					/// request to quit
		CACHE_ALIGN sys::atomic32_t volatile m_wr_idx = 0;		/// write index
		typedef sys::MessagePool<sys::msg_t, 1024> pool_t;
		pool_t g_MessagePool;			                        /// pool of messages
		CACHE_ALIGN sys::atomic32_t volatile m_rd_idx = 0;		/// read index

		sys::Thread g_ThreadSend;		/// consumer-sender thread (high priority)
		int g_Socket = -1;
		sys::Timer g_ReconnectTimer;

		inline sys::msg_t & msg_buffer_at (size_t i) { return g_MessagePool[i]; }
		inline sys::msg_t & acquire_msg_buffer ()
		{
			return msg_buffer_at(sys::atomic_inc32(&m_wr_idx) % pool_t::e_size);
		}

		inline bool is_connected () { return g_Socket >= 0; }
		int get_errno () { return errno; }
		bool is_timeouted () { return errno == ETIMEDOUT; }

		inline bool WriteToSocket (char const * buff, size_t ln)
		{
			//for (size_t i = 0; i < msg.m_length; ++i) printf("%c ", msg.m_data[i] < 32 ? '.' msg.m_data[i]); printf("\n");
			if (is_connected())
			{
				int const result = send(g_Socket, buff, (int)ln, 0);
				if (result == -1)
				{
					DBG_OUT("send failed with error: %d\n", get_errno());
					close(g_Socket);
					g_Socket = -1;
					return false;
				}
			}
			return true;
		}

		bool try_connect ();

		/**@brief	function consuming and sending items from MessagePool **/
		void * consumer_thread ( void * )
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
		void connect (char const * host, char const * port_str)
		{
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd < 0)
			{
				perror("ERROR opening socket");
				return;
			}
			hostent * server = gethostbyname(host);
			if (server == NULL)
			{
				fprintf(stderr,"ERROR, no such host\n");
				return;
				exit(0);
			}
			sockaddr_in serv_addr;
			bzero((char *) &serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
			int const port = atoi(port_str);
			serv_addr.sin_port = htons(port);
			if (connect(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
			{
				perror("ERROR connecting");
				g_Socket = -1;
				return;
			}

			g_Socket = sockfd;

			//int send_buff_sz = 64 * 1024;
			//setsockopt(g_Socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char *>(&send_buff_sz), sizeof(int));
			//DWORD send_timeout_ms = 1;
			//setsockopt(g_Socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&send_timeout_ms), sizeof(DWORD));
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
	}

	void Disconnect ()
	{
		socks::g_Quit = 1; // @TODO: atomic_store?
		if (socks::is_connected())
		{
			close(socks::g_Socket);
			socks::g_Socket = -1;
		}
		socks::g_ThreadSend.WaitForTerminate();
		socks::g_ThreadSend.Close();
	}

	inline void WriteBgn_Impl ()
	{
		sys::msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_bgn(msg);
		}
		msg.WriteUnlockAndDirty();
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

	inline void WriteEnd_Impl ()
	{
		sys::msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_end(msg);
		}
		msg.WriteUnlockAndDirty();
	}
	inline void WriteEndVA (char const * fmt, va_list args)
	{
		sys::msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_end(msg, fmt, args);
		}
		msg.WriteUnlockAndDirty();
	}

	inline void WriteFrameBgn_Impl ()
	{
		sys::msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_frame_bgn(msg);
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

	inline void WriteFrameEnd_Impl ()
	{
		sys::msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_frame_end(msg);
		}
		msg.WriteUnlockAndDirty();
	}
	inline void WriteFrameEndVA (char const * fmt, va_list args)
	{
		sys::msg_t & msg = socks::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_frame_end(msg, fmt, args);
		}
		msg.WriteUnlockAndDirty();
	}

}

