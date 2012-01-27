#pragma once
#include <ws2tcpip.h>
#include "../../tlv_parser/tlv_parser.h"
#include "../../tlv_parser/tlv_encoder.h"

namespace trace {

	inline void encode_setup (sys::Message & msg, level_t level, context_t context)
	{
		tlv::Encoder e(tlv::cmd_setup, msg.m_data, sys::Message::e_data_sz);
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag_app,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%s", GetAppName()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%x", context), tlv_buff));
		e.Encode(TLV(tag_pid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", sys::get_pid()), tlv_buff));
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
}
