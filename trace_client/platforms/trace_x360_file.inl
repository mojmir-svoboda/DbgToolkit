#pragma once
#include <XmCore.h>
#include <Xbdm.h>
#include "trace_x360_common.inl"
#include "encode_log.inl"
#include "encode_scope.inl"
#include "encode_setup.inl"

namespace trace {
	namespace file {

		char const path[] = "e:\\log\\";
		char const name[] = "hammerheart";
		char const ext[] = "tlv_trace";
		XLOCKFREE_LOG g_File;

		void mk_name (char * buff, size_t ln)
		{
			_snprintf(buff, ln, "%s%s%llu.%s", path, name, sys::queryTime_ms(), ext);
		}

		void OpenFile ()
		{
			DmMapDevkitDrive();
			char fname[512];
			mk_name(fname, sizeof(fname) / sizeof(*fname));
			int const msg_size = 1024;
			int const msg_count = 512;
			XLFStartLog(0, fname, msg_size, msg_count, FALSE, &g_File);
		}

		void CloseFile ()
		{
			XLFEndLog(g_File);
		}

		void WriteToFile (char const * buff, size_t ln)
		{
			XLFLogPrint(g_File, "%s", buff);
		}
	}


	void Connect ()
	{
		sys::setTimeStart();

		file::OpenFile();

		// send cmd_setup message
		msg_t msg;
		encode_setup(msg, GetRuntimeLevel(), GetRuntimeContextMask());
		file::WriteToFile(msg.m_data, msg.m_length);
	}

	void Disconnect ()
	{
		file::CloseFile();
	}

	inline void WriteLog (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		msg_t msg;
		encode_log(msg, level, context, file, line, fn, fmt, args);
		file::WriteToFile(msg.m_data, msg.m_length);
	}

	inline void WriteStr (level_t level, context_t context, char const * file, int line, char const * fn, char const * str)
	{
		msg_t msg;
		encode_str(msg, level, context, file, line, fn, str);
		file::WriteToFile(msg.m_data, msg.m_length);
	}

	inline void WriteScope (ScopedLog::E_Type type, level_t level, context_t context, char const * file, int line, char const * fn)
	{
		msg_t msg;
		encode_scope(msg, type == ScopedLog::e_Entry ? tlv::cmd_scope_entry : tlv::cmd_scope_exit , level, context, file, line, fn);
		file::WriteToFile(msg.m_data, msg.m_length);
	}
}
