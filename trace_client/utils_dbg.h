#pragma once

void dbg_out (char const * format, ...)
{
	using clock_t = std::chrono::high_resolution_clock;
	using us = std::chrono::duration<long long, std::nano>;
	static clock_t::time_point start_us = clock_t::now();
	clock_t::time_point now_us = clock_t::now();
	us t = now_us - start_us;

	char buffer[256];
	size_t n = snprintf(buffer, 256, "%10lld|%8x|", std::chrono::duration_cast<std::chrono::microseconds>(t), std::this_thread::get_id());
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buffer + n, 256 - n, format, argptr);
	va_end(argptr);
	OutputDebugStringA(buffer);
}
