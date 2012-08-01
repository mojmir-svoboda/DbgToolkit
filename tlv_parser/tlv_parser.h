/**
 * Copyright (C) 2011 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once
#include <cstring>
#include <string>
#include <vector>

namespace tlv {

	typedef unsigned char cmd_t;					/// command type (1 byte for now)
	typedef unsigned char tag_t;					/// tag type (1 byte)
	typedef unsigned short len_t;					/// length type (2 bytes)

	/** available commands */
	static cmd_t const cmd_setup             = 0xFF;	/// trace setup message
	static cmd_t const cmd_log               = 0xFE;	/// trace log message
	static cmd_t const cmd_scope_entry       = 0xFD;	/// trace scope
	static cmd_t const cmd_scope_exit        = 0xFC;	/// trace scope
	static cmd_t const cmd_restart_req       = 0xFB;	/// tcp stream restart request
	static cmd_t const cmd_restart_ack       = 0xFA;	/// tcp stream restart ack
	static cmd_t const cmd_set_level         = 0xF9;	/// set debug level
	static cmd_t const cmd_set_level_ack     = 0xF8;	/// set debug level ack
	static cmd_t const cmd_set_ctx           = 0xF7;	/// adjust runtime context filtering
	static cmd_t const cmd_set_ctx_ack       = 0xF6;	/// request context from other party
	static cmd_t const cmd_set_buffering     = 0xF5;	/// set buffering on/off
	static cmd_t const cmd_export_csv        = 0xF4;	/// export as csv
	static cmd_t const cmd_save_tlv          = 0xF3;	/// save as tlv (native format)
	static cmd_t const cmd_setup_ack         = 0xF2;	/// acknowledge setup command
	static cmd_t const cmd_data_xy           = 0xF1;	/// draw data xy
	static cmd_t const cmd_data_xyz          = 0xF0;	/// draw data xyz
                                             
	static cmd_t const cmd_profile_bgn       = 0xEF;	/// send profiling begin
	static cmd_t const cmd_profile_end       = 0xEE;	/// send profiling end
	static cmd_t const cmd_profile_frame_bgn = 0xED;	/// send profiling frame begin
	static cmd_t const cmd_profile_frame_end = 0xEC;	/// send profiling frame end

	enum e_Tags
	{
		tag_invalid = 0,  /// invalid tag value
		tag_app,          /// application name
		tag_pid,          /// process id
		tag_time,         /// time
		tag_tid,          /// thread id
		tag_file,         /// file name
		tag_line,         /// line
		tag_func,         /// function
		tag_msg,          /// logged message
		tag_lvl,          /// logging level
		tag_ctx,          /// logging context

		/// now following tags are considered as "system"
		tag_bool,         /// bool
		tag_int,          /// int
		tag_string,       /// string
		tag_float,        /// float
		tag_x,            /// x
		tag_y,            /// y
		tag_z,            /// z

		tag_max_value     /** this should be last line of enum **/
	};

	/** printable form of tags */
	static char const * tag_names[tag_max_value] =
	{
		"---",
		"App",
		"PID",
		"Time",
		"TID",
		"File",
		"Line",
		"Func",
		"Msg",
		"Lvl",
		"Ctx",
		/*sys tags*/
		"Bool",
		"Int",
		"Str",
		"Flt",
		"x",
		"y",
		"z"
	};

	inline size_t get_tag_count () { return tag_max_value; }

	inline char const * get_tag_name (size_t i)
	{
		if (i < tag_max_value)
			return tag_names[i];
		return 0;
	}

	inline size_t tag_for_name (char const * tag_name)
	{
		for (size_t i = 0; i < tag_max_value; ++i)
			if (strcmp(tag_names[i], tag_name) == 0)
				return i;
		return tag_invalid;
	}

	/********************************************
	 * protocol structure is very simple:
	 * Header | Payload
	 * where Payload is a sequence of TLV's
	 * i.e.:
	 * Header | TLVTLVTLV.... TLV
	 ********************************************/
	struct Header
	{
		cmd_t cmd;  // 1 Byte command ( setup | log | ... )
		len_t len;  // 2 Byte length of payload

		enum { e_Size = sizeof(cmd_t) + sizeof(len_t) } ;

		Header (cmd_t c, len_t l) : cmd(c), len(l) { }
		void Reset () { cmd = 0; len = 0; }
	};

	//////////////////// char * oriented command /////////////////////
	struct TLV
	{
		tag_t        m_tag;	// 1 Byte tag
		len_t        m_len;	// 2 Byte length of payload
		char const * m_val;	// payload

		TLV () : m_tag(tag_invalid), m_len(0), m_val(0) { }
		TLV (tag_t t, char const * v) : m_tag(t) , m_len(static_cast<len_t>(strlen(v))) , m_val(v) { } // constructor requires 0 terminated string
		TLV (tag_t t, len_t l, char const * v) : m_tag(t) , m_len(l) , m_val(v) { }

		len_t len () const { return m_len; }
	};

	template<size_t N, size_t M>
	struct Command
	{
		static unsigned const max_buff_size = N;
		static unsigned const max_tlvs = M;

		typedef TLV tlvs_t[max_tlvs];		/// array of TLVs
		Header hdr;							/// header
		unsigned tlvs_count;				/// count of TLVs
		tlvs_t tlvs;						/// array of TLV
		char concat_values[max_buff_size];	/// internal concantenated values from list of TLV
	
		Command () : hdr(0, 0), tlvs_count(0) { }
		void Reset ()
		{
			hdr.Reset();
			tlvs_count = 0;
		}
	};

	//////////////////// std::string oriented command /////////////////////
	struct TV
	{
		tlv::tag_t        m_tag;	// 1 Byte tag
		std::string       m_val;	// payload

		TV (tag_t t, char const * v) : m_tag(t) , m_val(v) { }
		TV () : m_tag(0) , m_val() { }
		len_t len () const { return static_cast<len_t>(m_val.length()); }
	};
	typedef std::vector<TV> tvs_t;

	struct StringCommand
	{
		tlv::Header hdr;
		tvs_t tvs;

		StringCommand () : hdr(0, 0) { }

		void Reset ()
		{
			hdr.Reset();
			tvs.clear();
		}
	};
}

