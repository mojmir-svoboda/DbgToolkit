#pragma once
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_decoder.h>
#include "trace_linux_common.inl"
#include "encode_log.inl"
#include "encode_scope.inl"
#include "encode_setup.inl"

//#define DBG_OUT printf
#define DBG_OUT(fmt, ...) ((void)0)

namespace trace {

	namespace socks {

		sys::atomic32_t volatile g_Quit = 0;			/// request to quit
		CACHE_ALIGN sys::atomic32_t volatile m_wr_idx = 0;		// write index
		sys::MessagePool g_MessagePool;					// pool of messages
		CACHE_ALIGN sys::atomic32_t volatile m_rd_idx = 0;		// read index

		sys::Thread g_ThreadSend;		/// consumer-sender thread (high priority)
		sys::Thread g_ThreadRecv;		/// receiving thread (low priority)
		int g_Socket = -1;
		FILE * g_LogFile = 0;
		sys::Timer g_ReconnectTimer;
		sys::Timer g_ClottedTimer;

		inline sys::Message & msg_buffer_at (size_t i)
		{
			return g_MessagePool[i];
		}

		inline sys::Message & acquire_msg_buffer ()
		{
			sys::atomic32_t wr_idx = sys::atomic_inc32(&m_wr_idx);
			return msg_buffer_at((wr_idx - 1) % sys::MessagePool::e_size);
		}

		inline bool is_connected () { return g_Socket >= 0; }
		inline bool is_file_connected () { return g_LogFile != 0; }

		inline bool WriteToFile (char const * buff, size_t ln)
		{
			if (is_file_connected())
			{
				fwrite(buff, 1, ln, g_LogFile);
				return true;
			}
			return false;
		}

		int get_errno () { return errno; }
		bool is_timeouted () { return errno == ETIMEDOUT; }

		inline bool WriteToSocket (char const * buff, size_t ln)
		{
			if (is_connected())
			{
				int result = -1;
				if (!g_ClottedTimer.enabled())
					result = send(g_Socket, buff, (int)ln, 0);
				else
				{
					if (g_ClottedTimer.expired())
					{
						result = send(g_Socket, buff, (int)ln, 0);
						if (result != -1 && result > 0)
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

				bool const timeouted = ( result == -1 && is_timeouted());
				if (result == -1 && !timeouted)
				{
					DBG_OUT("send failed with error: %d\n", get_errno());
					close(g_Socket);
					g_Socket = -1;

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
		void * receive_thread ( void * )
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
					size_t const n_bytes = result;
					if (d.decode_header<256, 8>((char const *)(&buff[0]), n_bytes, cmd))
					{
						if (d.decode_payload<256, 8>(&buff[0] + tlv::Header::e_Size, cmd.hdr.len, cmd))
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
		void * consumer_thread ( void * )
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
				sys::atomic32_t wr_idx = sys::atomic_get(&m_wr_idx);
				sys::atomic32_t rd_idx = m_rd_idx;
				// @TODO: wraparound
				if (rd_idx < wr_idx)
				{
					//DBG_OUT("rd_idx=%10i, wr_idx=%10i, diff=%10i \n", rd_idx, wr_idx, wr_idx - rd_idx);
					sys::Message & msg = socks::msg_buffer_at(rd_idx % sys::MessagePool::e_size);
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
			g_ClottedTimer.reset();
			sys::Message msg;
			// send cmd_setup message
			encode_setup(msg, GetRuntimeLevel(), GetRuntimeContextMask());

			socks::connect("localhost", "13127");

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
		sys::SetTickStart();

		socks::g_ThreadSend.Create(socks::consumer_thread, 0);

		bool const connected = socks::try_connect();
		if (!connected)
			socks::g_ReconnectTimer.set_delay_ms(1000);

#	if defined TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
		char filename[128];
		sys::create_log_filename(filename, sizeof(filename) / sizeof(*filename));
		//socks::g_LogFile = fopen(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		socks::g_LogFile = fopen(filename, "w");

		sys::Message msg;
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
			close(socks::g_Socket);
			socks::g_Socket = -1;
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

