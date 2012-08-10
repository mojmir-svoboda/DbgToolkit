#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>

namespace trace {

	inline void encode_shutdown (msg_t & msg)
	{
		tlv::Encoder e(tlv::cmd_shutdown, msg.m_data, msg_t::e_data_sz);
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

