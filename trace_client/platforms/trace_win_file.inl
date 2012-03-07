#define WIN32_LEAN_AND_MEAN
//#define NOMINMAX
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "trace_win_common.inl"
#include "encode_log.inl"
#include "encode_scope.inl"
#include "encode_setup.inl"

namespace trace {

	namespace file {

		LONG volatile g_Quit = 0;			/// request to quit
		HANDLE g_LogFile = INVALID_HANDLE_VALUE;
#if defined TRACE_WINDOWS_USES_MEMMAP
		HANDLE g_MemMap = INVALID_HANDLE_VALUE;
		char * g_View = 0;
#endif
		CACHE_ALIGN LONG volatile m_wr_idx = 0;		// write index
		sys::MessagePool g_MessagePool;				// pool of messages
		CACHE_ALIGN LONG volatile m_rd_idx = 0;		// read index

		sys::Thread g_ThreadSend(THREAD_PRIORITY_HIGHEST);		/// consumer-sender thread (high priority)

		inline sys::Message & msg_buffer_at (size_t i)
		{
			return g_MessagePool[i];
		}

		inline sys::Message & acquire_msg_buffer ()
		{
			LONG wr_idx = InterlockedIncrement(&m_wr_idx);
			return msg_buffer_at(wr_idx % sys::MessagePool::e_size);
		}

		bool is_connected () { return g_LogFile != INVALID_HANDLE_VALUE; }

		bool WriteToFile (char const * buff, size_t ln)
		{
#if defined TRACE_WINDOWS_USES_MEMMAP
			if (g_View != INVALID_HANDLE_VALUE)
			{
				memcpy(file::g_View, buff, ln);
				file::g_View += ln;
				return true;
			}
#else
			if (g_LogFile != INVALID_HANDLE_VALUE)
			{
				DWORD written;
				WriteFile(g_LogFile, buff, ln, &written, 0);
				return true;
			}
#endif
			return false;
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
					sys::Message & msg = file::msg_buffer_at(rd_idx % sys::MessagePool::e_size);
					msg.ReadLock();

					bool const write_ok = file::WriteToFile(msg.m_data, msg.m_length);
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

	void Connect ()
	{
		char filename[128];
		sys::create_log_filename(filename, sizeof(filename) / sizeof(*filename));

		file::g_LogFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file::g_LogFile == INVALID_HANDLE_VALUE)
		{
			printf("cannot create log...\n");
			return;
		}
#if defined TRACE_WINDOWS_USES_MEMMAP
		size_t const mmapsize = 0x2000000;
		file::g_MemMap = CreateFileMapping(file::g_LogFile, NULL, PAGE_READWRITE, 0, mmapsize, NULL); //CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL)) != (HANDLE)I
		if (file::g_MemMap == INVALID_HANDLE_VALUE)
		{	
			printf("cannot create memory map...\n");
			CloseHandle(file::g_LogFile);
			return;
		}
		file::g_View = static_cast<char *>(MapViewOfFile (file::g_MemMap, FILE_MAP_WRITE, 0, 0, 0));
#endif

		if (file::is_connected())
		{
			file::g_ThreadSend.Create(file::consumer_thread, 0);
			sys::SetTickStart();

			sys::Message msg;
			encode_setup(msg, GetRuntimeLevel(), GetRuntimeContextMask());
			if (msg.m_length)
			{
				file::WriteToFile(msg.m_data, msg.m_length);
			}

			file::g_ThreadSend.Resume();
		}
	}

	void Disconnect ()
	{
#if defined TRACE_WINDOWS_USES_MEMMAP
		//FlushViewOfFile(file::g_MemMap, size);
		CloseHandle(file::g_MemMap);
#endif
		CloseHandle(file::g_LogFile);

		file::g_Quit = 1; // @TODO: atomic_store
		file::g_ThreadSend.WaitForTerminate();
		file::g_ThreadSend.Close();
	}

	inline void WriteLog (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		sys::Message & msg = file::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_log(msg, level, context, file, line, fn, fmt, args);
		}
		msg.WriteUnlockAndDirty();
	}

	inline void WriteScope (ScopedLog::E_Type type, level_t level, context_t context, char const * file, int line, char const * fn)
	{
		sys::Message & msg = file::acquire_msg_buffer();
		msg.WriteLock();
		{
			encode_scope(msg, type == ScopedLog::e_Entry ? tlv::cmd_scope_entry : tlv::cmd_scope_exit, level, context, file, line, fn);
		}
		msg.WriteUnlockAndDirty();
	}
}
