#pragma once

namespace trace {

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

	// Scopes
	inline void WriteScopeVA (ScopedLog::E_Type type, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		if (GetRuntimeBuffering())
		{
			msg_t & msg = socks::acquire_msg_buffer();
			msg.WriteLock();
			{
				encode_scope(msg, type == ScopedLog::e_Entry ? tlv::cmd_scope_entry : tlv::cmd_scope_exit , level, context, file, line, fn, fmt, args);
			}
			msg.WriteUnlockAndDirty();
		}
		else
		{
			msg_t msg;
			encode_scope(msg, type == ScopedLog::e_Entry ? tlv::cmd_scope_entry : tlv::cmd_scope_exit , level, context, file, line, fn, fmt, args);
			socks::WriteToSocket(msg.m_data, msg.m_length);
		}
	}

	// Data
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

	// Table
	inline void WriteTable_impl (level_t level, context_t context, int x, int y, char const * fmt, va_list args)
	{
		if (GetRuntimeBuffering())
		{
			msg_t & msg = socks::acquire_msg_buffer();
			msg.WriteLock();
			{
				encode_table(msg, level, context, x, y, fmt, args);
			}
			msg.WriteUnlockAndDirty();
		}
	}

	inline void SetCustomUserDictionnary (CtxDictPair const * ptr, size_t n)
	{
		if (GetRuntimeBuffering())
		{
			msg_t & msg = socks::acquire_msg_buffer();
			msg.WriteLock();
			{
				encode_ctx_dict(msg, ptr, n);
			}
			msg.WriteUnlockAndDirty();
		}
		else
		{
			msg_t msg;
			encode_ctx_dict(msg, ptr, n);
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
}

