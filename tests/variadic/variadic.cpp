#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>   // for vsnprintf etc
#include "../../tlv_parser/tlv_parser.h"
#include <stdarg.h>	// for va_args

#	define TRACE_MSG(fmt, ... )	trace::Write(fmt, __VA_ARGS__);
#	define TRACE_VA(fmt, vaargs) trace::Write(fmt, vaargs);

namespace trace {

	typedef unsigned level_t;
	typedef unsigned context_t;

	inline void encode_str (char const * fmt, char const * args)
	{
		printf("\tencode_str %s\n", __FUNCTION__);
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];

		int const n = _snprintf(tlv_buff, tlv_buff_sz, fmt, args);
        int const wrt_n = n < 0 ? tlv_buff_sz : n;
		printf("\t   STR LOG: %s", tlv_buff);
	}

	inline void encode_log (char const * fmt, va_list args)
	{
		printf("\tencode_log %s\n", __FUNCTION__);
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;

		  int const n = vsnprintf(tlv_buff, tlv_buff_sz, fmt, args);
		printf("\tCHAR * LOG: %s", tlv_buff);
	}


    inline void WriteLog (char const * fmt, va_list args)
    {       
		printf("\tva_list \t%s\n", __FUNCTION__);
		encode_log(fmt, args);
    }

    inline void WriteLog (char const * fmt, char const * args)
    {       
		printf("char %s\n", __FUNCTION__);
		encode_str(fmt, args);
    }

	void Write (char const * fmt, va_list args)
	{
		printf("va_list %s\n", __FUNCTION__);
		WriteLog(fmt, args);
	}
	void Write (char const * fmt, ...)
	{
		printf("...     %s\n", __FUNCTION__);
		va_list args;
		va_start(args, fmt);
		Write(fmt, args);
		va_end(args);
	}

	// if following block is uncommented it stops crashing on msvc x64 build
	/*void Write (char const * fmt, char const * args)
	{
		printf("char *  %s\n", __FUNCTION__);
		WriteLog(fmt, args);
	}*/

}

int main ()
{
	printf("%s\n", __FUNCTION__);
	TRACE_MSG("args: %s and %s\n\n", "first arg", "second arg");
	TRACE_MSG("args: %s\n\n", "first arg");
	TRACE_MSG("args: %u\n\n", 1);
}
