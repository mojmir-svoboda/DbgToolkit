#pragma once
#include <cstring>
#include <string>
#include <vector>

namespace tlv {

	typedef unsigned char cmd_t;					/// command type (1 byte for now)
	typedef unsigned char tag_t;					/// tag type (1 byte)
	typedef unsigned short len_t;					/// length type (2 bytes)

	/** available commands */
	static cmd_t const cmd_setup         = 0xFF;	/// trace setup message
	static cmd_t const cmd_log           = 0xFE;	/// trace log message
	static cmd_t const cmd_scope_entry   = 0xFD;	/// trace scope
	static cmd_t const cmd_scope_exit    = 0xFC;	/// trace scope
	static cmd_t const cmd_restart_req   = 0xFB;	/// tcp stream restart request
	static cmd_t const cmd_restart_ack   = 0xFA;	/// tcp stream restart ack
	static cmd_t const cmd_set_level     = 0xF9;	/// set debug level
	static cmd_t const cmd_set_level_ack = 0xF8;	/// set debug level ack
	static cmd_t const cmd_set_ctx       = 0xF7;	/// adjust runtime context filtering
	static cmd_t const cmd_set_ctx_ack   = 0xF6;	/// request context from other party

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
		/** @NOTE: add new tags here **/
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
		/** @NOTE: add new tags here **/
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

		TLV (tag_t t, char const * v) : m_tag(t) , m_len(static_cast<len_t>(strlen(v))) , m_val(v) { } // constructor requires 0 terminated string
		TLV (tag_t t, len_t l, char const * v) : m_tag(t) , m_len(l) , m_val(v) { }

		len_t len () const { return m_len; }
	};
	typedef std::vector<TLV> tlvs_t;

	template<size_t N>
	struct Command
	{
		Header hdr;		/// header
		tlvs_t tlvs;	/// array of TLV

		Command (size_t n)
			: hdr(0, 0)
		{
			tlvs.reserve(n);
		}
		void Reset ()
		{
			hdr.Reset();
			tlvs.clear();
		}

		static unsigned const max_buff_size = N;
		char concat_values[max_buff_size];	/// internal concantenated values from list of TLV
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

