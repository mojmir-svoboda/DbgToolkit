#pragma once
#define WIN32_LEAN_AND_MEAN
#if defined __MINGW32__
#	undef _WIN32_WINNT
#	define _WIN32_WINNT 0x0600 
#	include <windows.h>
#endif
#include <cstdio>
#include <tlv_parser/tlv_parser.h>
#include <sysfn/select_atomic.h>
#include <sysfn/time_query.h>
#include <sysfn/os.h>
#include "msg.h"

namespace profile {

	void create_log_filename (char * filename, size_t buff_sz)
	{
		char const * app_name = GetAppName() ? GetAppName() : "unknown";
		_snprintf_s(filename, buff_sz, buff_sz - 1, "%s_%u.tlv_trace", app_name, ::GetCurrentProcessId());
	}

	typedef sys::Message<192> msg_t;

	/**@brief	simple pool of messages to be logged **/
	template <class T, unsigned N = 512>
	struct MessagePool
	{
		enum { e_size = N };
		CACHE_ALIGN msg_t m_msgs[e_size];

		MessagePool () { memset(this, 0, sizeof(*this)); }
		msg_t & operator[] (size_t i) { return m_msgs[i]; }
		msg_t const & operator[] (size_t i) const { return m_msgs[i]; }
	};
}

