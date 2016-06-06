#pragma once

struct Stats {

	size_t m_received_bytes { 0 };
	size_t m_received_cmds { 0 };
	size_t m_received_failed_cmds { 0 };
	size_t m_received_batches { 0 };
	size_t m_decoder_mem_asn1 { 0 };
	size_t m_decoder_mem_asn1_realloc_count { 0 };
	size_t m_decoder_mem_recv_buff { 0 };

	void updateDecoderMemAsn1Max (size_t val) { if (val > m_decoder_mem_asn1) m_decoder_mem_asn1 = val; }
	void updateDecoderMemRecvBuffMax (size_t val) { if (val > m_decoder_mem_recv_buff) m_decoder_mem_recv_buff = val; }

	static Stats & get ()
	{
		static Stats instance;
		return instance;
	}

	~Stats ()
	{
	}
};



