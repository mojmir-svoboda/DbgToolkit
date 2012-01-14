#pragma once
#include <cstring>
#include <vector>
#include "memstream.h"

namespace tlv {

	/**@class	Encoder
	 * @brief	main encoder class
	 */
	struct Encoder
	{
		len_t total_len;		/// total message length including header
		len_t data_len;			/// payload (data) length (i.e. without header)
		memstream output;		/// encapsulation of raw buffer
		char * buffer;			///	raw buffer


		Encoder (cmd_t cmd, char * buff, size_t ln) 
			: total_len(0) , data_len(0) , output(buff, ln) , buffer(buff) 
		{
			Header h(cmd, 0);
			Encode(h);
		}

		bool Commit ()
		{
			bool const is_ok = !output.write_eof();
			if (is_ok)
			{
				total_len = static_cast<len_t>(output.total_write_len());
				data_len = total_len - Header::e_Size;
                len_t const encoded_ln = htons(data_len);
                buffer[sizeof(cmd_t)] = reinterpret_cast<char const *>(&encoded_ln)[0];
                buffer[sizeof(cmd_t) + 1] = reinterpret_cast<char const *>(&encoded_ln)[1]; // write length of data
			}
			return is_ok;
		}

		void Encode (Header h)
		{
			output.write(reinterpret_cast<char const * >(&h.cmd), sizeof(cmd_t));
			output.write(reinterpret_cast<char const * >(&h.len), sizeof(len_t));
		}

		void Encode (tlvs_t const & tlvs)
		{
			for (tlvs_t::const_iterator it = tlvs.begin(), ite = tlvs.end(); it != ite; ++it)
				Encode(*it);
		}

		void Encode (TLV const & tlv)
		{
			output.write(reinterpret_cast<char const * >(&tlv.m_tag), sizeof(tag_t));
			output.write(reinterpret_cast<char const * >(&tlv.m_len), sizeof(len_t));
			output.write(reinterpret_cast<char const * >(&tlv.m_val[0]), tlv.len());
		}
	};
}

