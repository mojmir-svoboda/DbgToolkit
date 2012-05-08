#pragma once
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE 1
#endif
#include <ctype.h>
#include <errno.h>
#include <cstdio>	// for vsnprintf etc
#include <cstdlib>	// for atoi
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "time_query.h"
#include "os.h"
#include "msg.h"

namespace trace {

	void create_log_filename (char * filename, size_t buff_sz)
	{
		char const * app_name = GetAppName() ? GetAppName() : "unknown";
		snprintf(filename, buff_sz, "%s_%u.tlv_trace", app_name, sys::get_pid());
	}

	typedef sys::Message<1024> msg_t;

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

