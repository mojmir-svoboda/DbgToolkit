#include "trace.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <trace_proto/header.h>
#include <trace_proto/decoder.h>
#define NOMINMAX 1
#include <asio.hpp>
#include <sysfn/time_query.h>

#define DBG_CLIENT 1
#if !defined DBG_CLIENT
#define DBG_OUT(fmt, ...) ((void)0)
#else
void dbg_out (char const * format, ...)
{
	using clock_t = std::chrono::high_resolution_clock;
	using us = std::chrono::duration<long long, std::nano>;
	static clock_t::time_point start_us = clock_t::now();
	clock_t::time_point now_us = clock_t::now();
	us t = now_us - start_us;

	char buffer[256];
	size_t n = snprintf(buffer, 256, "%10lld|%8x|", std::chrono::duration_cast<std::chrono::microseconds>(t), std::this_thread::get_id());
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buffer + n, 256 - n, format, argptr);
	va_end(argptr);
	OutputDebugStringA(buffer);
}
#define DBG_OUT(fmt, ...) dbg_out(fmt, __VA_ARGS__)
#endif

namespace trace {

	void OnConnectionEstablished ();
	void OnConnectionConfigCommand (Command const & cmd);

	struct SpinLock
	{
		std::atomic_flag m_lock { ATOMIC_FLAG_INIT };

		void Lock ()
		{
			while (m_lock.test_and_set(std::memory_order_acquire))
				;
		}
		
		void Unlock ()
		{
			m_lock.clear(std::memory_order_release);
		}
	};

	//using buffptr_t = std::atomic<size_t>;
	struct MsgHeader
	{
		uint32_t m_size { 0 };
		uint32_t m_allocated { 0 };
	};

	struct ClientMemory
	{
		//buffptr_t m_readPtr;
		//buffptr_t m_writePtr;
		SpinLock m_lock;
		uint32_t m_readPtr { 0 };
		uint32_t m_writePtr { 0 };

		enum : size_t { e_memSize = 1024 * 1024 };
		enum : size_t { e_memAlign = 64 };
		alignas(e_memAlign) char m_memory[e_memSize];

		ClientMemory ()
		{
			//static_assert(alignof(m_readPtr) == 16, "problem");
			//m_readPtr.store(0, std::memory_order_relaxed);
			//m_writePtr.store(0, std::memory_order_relaxed);
		}

		char * AcquireMem (size_t n)
		{
			char * mem = nullptr;
			m_lock.Lock();

			size_t const curr_wr_ptr = m_writePtr; //m_writePtr.load(std::memory_order_acquire);
			size_t curr_space = e_memSize - curr_wr_ptr;
			void * const curr_mem = m_memory + curr_wr_ptr;
			void * next_mem = m_memory + curr_wr_ptr + n + sizeof(MsgHeader);
			if (std::align(e_memAlign, n, next_mem, curr_space))
			{
				mem = static_cast<char *>(curr_mem);
				char * const nmem = static_cast<char *>(next_mem);
				size_t const aligned_sz = nmem - mem;
				m_writePtr += aligned_sz;
				MsgHeader * hdr = new (mem) MsgHeader;
				hdr->m_allocated = aligned_sz;
			}

			m_lock.Unlock();
			return mem;
		}
	};

	ClientMemory g_ClientMemory;

	struct Client
	{
		bool m_terminated { false };
		bool m_connected { false };
		bool m_buffered { false };
		long long m_reconnect_ms { 3000 };
		unsigned m_reconnect_retry_count_max { 3 };
		unsigned m_reconnect_retry_count { 0 };
		unsigned m_asn1_buffer_sz { 16384 };
		asio::io_context m_io;
		std::unique_ptr<asio::io_service::work> m_work;
		asio::ip::tcp::socket m_socket;
		std::thread m_thread;
		asio::steady_timer m_timer;
		asio::ip::tcp::resolver::results_type m_endpoints;
		Asn1Allocator m_asn1_allocator;
		DecodingContext m_dcd_ctx;

		Client ()
			: m_timer(m_io)
			, m_socket(m_io)
		{
			DBG_OUT("Client created\n");
			m_asn1_allocator.resizeStorage(m_asn1_buffer_sz);
		}

		bool IsConnected () const { return m_connected; }
		void SetClientTimer (long long ms) { m_timer.expires_from_now(std::chrono::milliseconds(ms)); }
		void CancelClientTimer () { m_timer.cancel(); }
		void OnClientTimer ()
		{
			if (m_terminated)
				return;

			if (m_connected)
				return;

			if (m_timer.expires_at() <= std::chrono::steady_clock::now())
			{
				DBG_OUT("Reconnect expired\n");
				asio::error_code ignored_ec;
				m_socket.close(ignored_ec);

				StartReconnect();
			}

			StartClientTimer();
		}

		void StartReconnect ()
		{
			m_connected = false;
			if (m_terminated)
				return;

			DBG_OUT("Reconnect started\n");
			SetClientTimer(m_reconnect_ms);
			asio::async_connect(m_socket, m_endpoints, std::bind(&Client::OnConnect, this, std::placeholders::_1, m_endpoints));

			++m_reconnect_retry_count;
// 			if (m_reconnect_retry_count >= m_reconnect_retry_count_max)
// 			{
// 				DBG_OUT("Bad luck..\n");
// 			}
		}

		void StartClientTimer ()
		{
			if (m_terminated)
				return;
			DBG_OUT("StartClientTimer\n");
			m_timer.async_wait(std::bind(&Client::OnClientTimer, this));
		}

		void OnConnected ()
		{
			DBG_OUT("Connected!\n");
			m_connected = true;
			StartAsyncRead();
			OnConnectionEstablished();
		}

		bool Init (char const * host, char const * port)
		{
			try
			{
				asio::ip::tcp::resolver resolver(m_io);
				m_endpoints = resolver.resolve(host, port);
				if (bool const connected = DoConnect(m_endpoints))
				{
					OnConnected();
					const bool stopped = m_io.stopped();
					if (stopped)
						m_io.restart();
				}
				else
				{
					DBG_OUT("Cannot connect, will retry...\n");
					const bool stopped = m_io.stopped();
					if (stopped)
						m_io.restart();
					StartReconnect();
					StartClientTimer();
				}

				m_work.reset(new asio::io_service::work(m_io));
				m_thread = std::thread(&Client::ThreadFunc, this);
				return true;
			}
			catch (std::exception & e)
			{
				DBG_OUT(e.what());
				return false;
			}
		}

		void OnConnect (asio::error_code const & ec, asio::ip::tcp::resolver::results_type::iterator endpoint_iter)
		{
			if (m_terminated)
				return;

			if (!m_socket.is_open())
			{
				DBG_OUT("OnConnect: socket not opened in time, reconnecting\n");
				StartReconnect();
			}
			else if (ec)
			{
				DBG_OUT("OnConnect: reconnecting because of error, code=%d msg=%s\n", ec.value(), ec.message().c_str());
				OnSocketError(ec);
				StartReconnect();
			}
			else
			{
				DBG_OUT("OnConnect: connected\n");
				CancelClientTimer();
				OnConnected();
			}
		}

		bool DoConnect (asio::ip::tcp::resolver::results_type const & endpoints)
		{
			m_connected = false;
			asio::error_code result = asio::error::would_block;

			asio::async_connect(m_socket, endpoints,
				[&result](std::error_code ec, asio::ip::tcp::endpoint ep)
				{
					result = ec;
				});

			do
			{
				m_io.run_one();
			} while (result == asio::error::would_block); // sync wait for first connect

			if (result || !m_socket.is_open())
				return false;
			return true;
		}


		void ThreadFunc ()
		{
			DBG_OUT("Started thread for io\n");
			m_io.run();
			DBG_OUT("Terminated thread for io\n");
		}

		void Done ()
		{
			Flush();
			m_terminated = true;
			m_work.reset();
			m_timer.cancel();
			m_socket.close();
			m_thread.join();
		}

		void Flush ()
		{
		}

		void OnAsyncWrite(std::error_code ec, std::size_t ln)
		{
			DBG_OUT("OnAsyncWrite sz=%i ec=0x%x\n", ln, ec);
			if (!ec)
			{
				AsyncWriteNext();
			}
			else
			{
				OnSocketError(ec);
				StartReconnect();
				StartClientTimer();
			}
		}

		void AsyncWriteNext ()
		{
			DBG_OUT("AsyncWriteNext\n");
			m_pendingWrite.store(true, std::memory_order_release);

			g_ClientMemory.m_lock.Lock();
			size_t const rd_ptr = g_ClientMemory.m_readPtr;
			char const * const mem = g_ClientMemory.m_memory + rd_ptr;
			MsgHeader const * const hdr = reinterpret_cast<MsgHeader const *>(mem);
			asio::async_write(m_socket, asio::buffer(mem + sizeof(MsgHeader), hdr->m_size),
				std::bind(&Client::OnAsyncWrite, this, std::placeholders::_1, std::placeholders::_2));
			size_t const new_rd_ptr = rd_ptr + hdr->m_allocated;
			g_ClientMemory.m_writePtr = new_rd_ptr;
			g_ClientMemory.m_lock.Unlock();
		}


		bool WriteToSocket (char const * request, size_t ln)
		{
			return Write(request, ln);
		}

		bool Write (char const * buff, size_t ln)
		{
			if (IsConnected())
			{
				if (m_buffered)
				{
					char * buff_ptr = nullptr;
					if (WriteToBuffer(buff, ln, buff_ptr))
					{
// 						const bool has_pending = m_pendingWrite.load(std::memory_order_acquire);
// 						if (!has_pending)
// 							StartAsyncWrite();
					}
					else
					{
						// no room, spin/block
					}
				}
				else
				{
					std::error_code ec;
					asio::write(m_socket, asio::buffer(buff, ln), ec);
					if (ec)
					{
						OnSocketError(ec);
						StartReconnect();
						StartClientTimer();
					}
				}
			}
			else
			{
				char * buff_ptr = nullptr;
				if (!WriteToBuffer(buff, ln, buff_ptr))
				{
				}
			}
			return true;
		}

		char * AcquireBufferMem (size_t ln)
		{
			char * mem = g_ClientMemory.AcquireMem(ln);
			return mem;
		}

// 		char * ReleaseBufferMem (char const * buff, size_t ln)
// 		{
// 		}

		bool WriteToBuffer (char const * msg, size_t ln, char * & buff_ptr)
		{
			char * mem = AcquireBufferMem(ln);
			if (mem)
			{
				memcpy(mem + sizeof(MsgHeader), msg, ln);
				MsgHeader * hdr = reinterpret_cast<MsgHeader *>(mem);
				hdr->m_size = ln;
				//ReleaseBufferMem(mem, ln);
				buff_ptr = mem + sizeof(MsgHeader);
				return true;
			}
			buff_ptr = nullptr;
			return false;
		}

		void OnConnectFlush ()
		{
			if (m_buffered)
			{
// 				const bool has_pending = m_pendingWrite.load(std::memory_order_acquire);
// 				if (!has_pending)
// 					AsyncWriteNext();
			}
			else
			{
				DBG_OUT("Flush on connect (sync)\n");
			g_ClientMemory.m_lock.Lock();

			while (g_ClientMemory.m_readPtr != g_ClientMemory.m_writePtr)
			{
				char const * const mem = g_ClientMemory.m_memory + g_ClientMemory.m_readPtr;
				MsgHeader const * const hdr = reinterpret_cast<MsgHeader const *>(mem);
				std::error_code ec;
				asio::write(m_socket, asio::buffer(mem + sizeof(MsgHeader), hdr->m_size), ec);
				if (ec)
				{
					OnSocketError(ec);
					StartReconnect();
					StartClientTimer();
				}
				else
				{
					g_ClientMemory.m_readPtr += hdr->m_allocated;
				}
			}
			g_ClientMemory.m_lock.Unlock();
		}
		}

		void Close ()
		{
			DBG_OUT("Cli::Close\n");
			asio::post(m_io, [this]() { m_socket.close(); });
		}

		void OnSocketError (std::error_code const & ec)
		{
			DBG_OUT("OnSocketError: code=%d msg=%s\n", ec.value(), ec.message().c_str());
			m_connected = false;
			if (m_socket.is_open())
			{
				DBG_OUT("OnSocketError: closing socket.\n");
				m_socket.close();
			}
		}

		void StartAsyncRead ()
		{
			DBG_OUT("Starting async read.\n");
			DoReadHeader ();
		}

		void DoReadHeader ()
		{
			asio::async_read(m_socket,
				asio::buffer(m_dcd_ctx.getEndPtr(), sizeof(asn1::Header)),
				[this](std::error_code ec, std::size_t length)
			{
				if (!ec && length == sizeof(asn1::Header))
				{
					m_dcd_ctx.m_has_hdr = true;
					if (m_dcd_ctx.canMoveEnd(sizeof(asn1::Header)))
					{
						m_dcd_ctx.moveEnd(sizeof(asn1::Header));
						DoReadPayload();
					}
					else
					{
						// problem
					}
				}
				else
				{
					OnSocketError(ec);
					StartReconnect();
					StartClientTimer();
				}
			});
		}

		void DoReadPayload ()
		{
			DBG_OUT("Starting async read of payload.\n");
			size_t const payload_sz = m_dcd_ctx.getHeader().m_len;
			if (m_dcd_ctx.available() < payload_sz)
				OnSocketError(asio::error::no_buffer_space);

			asio::async_read(m_socket,
				asio::buffer(m_dcd_ctx.getEndPtr(), payload_sz),
				[this](std::error_code ec, std::size_t length)
			{
				if (!ec && length == m_dcd_ctx.getHeader().m_len)
				{
					if (m_dcd_ctx.canMoveEnd(length))
					{
						m_dcd_ctx.moveEnd(length);
						parseASN1();
						m_dcd_ctx.resetCurrentCommand();
						m_asn1_allocator.Reset();
						StartAsyncRead();
					}
					else
					{
						// problem
					}
				}
				else
				{
					OnSocketError(ec);
					StartReconnect();
					StartClientTimer();
				}
			});
		}

		bool parseASN1 ()
		{
			asn1::Header const & hdr = m_dcd_ctx.getHeader();
			if (hdr.m_version == 1)
			{
				Command * cmd_ptr = &m_dcd_ctx.m_command;
				void * cmd_void_ptr = cmd_ptr;
				char const * payload = m_dcd_ctx.getPayload();
				size_t const av = m_asn1_allocator.available();
				size_t const size_estimate = hdr.m_len * 4; // at most
				if (av < size_estimate)
				{	// not enough memory for asn1 decoder
					assert(0);
				}

				asn_dec_rval_t const rval = ber_decode(&m_asn1_allocator, 0, &asn_DEF_Command, &cmd_void_ptr, payload, hdr.m_len);
				if (rval.code != RC_OK)
				{
					m_dcd_ctx.resetCurrentCommand();
					m_asn1_allocator.Reset();
					assert(0);
					return false;
				}

				if (m_dcd_ctx.m_command.present == Command_PR_config)
				{
					OnConnectionConfigCommand(m_dcd_ctx.m_command);
					OnConnectFlush();
					DBG_OUT("reconnect ok, buffered data send\n");
				}
				else
				{
					assert(0);
				}
			}

			m_dcd_ctx.resetCurrentCommand();
			m_asn1_allocator.Reset();
			return true;
		}
	};

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
