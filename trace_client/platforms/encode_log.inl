#pragma once
#include <ws2tcpip.h>
#include "../../tlv_parser/tlv_parser.h"
#include "../../tlv_parser/tlv_encoder.h"

namespace trace {

	inline void encode_log (sys::Message & msg, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		tlv::Encoder e(tlv::cmd_log, msg.m_data, sys::Message::e_data_sz);
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag_time, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::GetTime()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", GetCurrentThreadId()), tlv_buff));
		e.Encode(TLV(tag_file, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%s", file), tlv_buff));
		e.Encode(TLV(tag_line, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", line), tlv_buff));
		e.Encode(TLV(tag_func, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%s", fn), tlv_buff));
		// this block is different due to weird bug in passing va_list from fn to fn (probably bad overload is chosen)
		// happens when calling stuff like:
		//		//TRACE(trace::e_Info, trace::CTX_Default,	"this is %s", "first message");
		//
#if defined __MINGW32__
		int const n = vsnprintf(tlv_buff, tlv_buff_sz, fmt, args);
#else
		int const n = _vsnprintf_s(tlv_buff, tlv_buff_sz, tlv_buff_sz - 1, fmt, args);
#endif
		int const wrt_n = n < 0 ? tlv_buff_sz : n;
		e.Encode(TLV(tag_msg,  static_cast<tlv::len_t>(wrt_n), tlv_buff));
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
}
