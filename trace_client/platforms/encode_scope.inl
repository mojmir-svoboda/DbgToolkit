#pragma once
#include <ws2tcpip.h>
#include "../tlv_parser/tlv_parser.h"
#include "../tlv_parser/tlv_encoder.h"
#include "../tlv_parser/tlv_decoder.h"
#pragma comment (lib, "Ws2_32.lib") // only for ntohs

namespace trace {

	inline void encode_scope (sys::Message & msg, tlv::cmd_t cmd, level_t level, context_t context, char const * file, int line, char const * fn)
	{
		tlv::Encoder e(cmd, msg.m_data, sys::Message::e_data_sz);
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
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
}
