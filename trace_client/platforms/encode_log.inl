#pragma once
#include "include_ntohs.h"
#include "../../tlv_parser/tlv_parser.h"
#include "../../tlv_parser/tlv_encoder.h"
#include "encode_common_fields.inl"

namespace trace {

	inline void encode_log (sys::Message & msg, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		using namespace tlv;
		tlv::Encoder e(tlv::cmd_log, msg.m_data, sys::Message::e_data_sz);
		encode_common_fields(e, level, context, file, line, fn);
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_str (sys::Message & msg, level_t level, context_t context, char const * file, int line, char const * fn, char const * str)
	{
		using namespace tlv;
		tlv::Encoder e(tlv::cmd_log, msg.m_data, sys::Message::e_data_sz);
		encode_common_fields(e, level, context, file, line, fn);
		encode_str(e, str);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
}
