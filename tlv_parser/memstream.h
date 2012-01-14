#pragma once
#include <cstring>

struct memstream
{
	char * m_buffer;
	size_t m_buff_sz;
	size_t m_rd_idx;
	size_t m_wr_idx;
	size_t m_state;

	enum {
		e_rd_eof   = 1 << 0,
		e_wr_eof   = 1 << 1,
	};

	memstream (char * buffer, size_t ln)		/// constructor for writing
		: m_buffer(buffer) , m_buff_sz(ln)
		, m_rd_idx(0) , m_wr_idx(0)
		, m_state(0)
	{ }

	memstream (char const * buffer, size_t ln)	/// constructor for reading
		: m_buffer(const_cast<char *>(buffer)) , m_buff_sz(ln)
		, m_rd_idx(0) , m_wr_idx(0)
		, m_state(0)
	{ }

	bool write_peek () const { return m_wr_idx < m_buff_sz; }
	bool read_peek () const { return m_rd_idx < m_buff_sz; }

	size_t write (char const * src, size_t ln)
	{
		int const free_size = m_buff_sz - m_wr_idx;
		size_t to_write = ln;
		if (m_wr_idx + ln > m_buff_sz)
		{
			to_write = free_size;
			m_state |= e_wr_eof;
		}

		if (to_write > 0)
		{
			memcpy(&m_buffer[m_wr_idx], src, to_write);
			m_wr_idx += to_write;
			return to_write;
		}

		m_state |= e_wr_eof;
		return 0;
	}

	size_t read (char * dst, size_t ln)
	{
		int const remaining = m_buff_sz - m_rd_idx;
		size_t to_read = ln;
		if (m_rd_idx + ln > m_buff_sz)
		{
			to_read = remaining;
			m_state |= e_rd_eof;
		}

		if (to_read > 0)
		{
			memcpy(dst, &m_buffer[m_rd_idx], to_read);
			m_rd_idx += to_read;
			return to_read;
		}

		m_state |= e_rd_eof;
		return 0;
	}

	size_t total_read_len () const { return m_rd_idx; }
	size_t total_write_len () const { return m_wr_idx; }

	bool read_eof () const { return (m_state & e_rd_eof) != 0; }
	bool write_eof () const { return (m_state & e_wr_eof) != 0; }
};
