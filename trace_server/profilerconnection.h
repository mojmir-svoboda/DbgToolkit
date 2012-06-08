#pragma once
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_decoder.h>
#include <filters/file_filter.hpp>
#include <boost/config.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <QObject>
#include "profilerblockinfo.h"
#include "rvps.h"
#include "cmd.h"

class MainWindow;

namespace profiler {

struct Connection : QObject, boost::enable_shared_from_this<profiler::Connection>
{
	Q_OBJECT
public:
	ProfileInfo m_profileInfo;
	boost::asio::io_service & m_io;
	boost::asio::ip::tcp::socket m_socket;
	DecodedCommand m_current_cmd;
	tlv::TVDecoder m_decoder;
	profiler_rvp_t & m_rvp;
	size_t m_last_flush_end_idx;
	MainWindow & m_main_window;

	explicit Connection (QObject * parent, boost::asio::io_service & io_service, profiler_rvp_t & rvp, MainWindow & mw);

	~Connection ()
	{
		//printf("--- connection \n");
		m_io.stop();
	}

	boost::asio::ip::tcp::socket & socket () { return m_socket; }

	void start ();

	void handle_read_header (boost::system::error_code const & error)
	{
		if (!error)
		{
			m_decoder.decode_header(&m_current_cmd.orig_message[0], tlv::Header::e_Size, m_current_cmd);
			if (m_current_cmd.hdr.cmd == 0 || m_current_cmd.hdr.len == 0)
			{
				return;
			}
	
			boost::asio::async_read(m_socket,
				boost::asio::buffer(&m_current_cmd.orig_message[tlv::Header::e_Size], m_current_cmd.hdr.len),
					boost::bind(&Connection::handle_read_payload, shared_from_this(), boost::asio::placeholders::error));
		}
		else
		{
			m_socket.close();
		}
	}

	void handle_read_payload (boost::system::error_code const & error)
	{
		if (!error)
		{
			if (m_decoder.decode_payload(&m_current_cmd.orig_message[tlv::Header::e_Size], m_current_cmd.hdr.len, m_current_cmd))
			{
				tryHandleCommand(m_current_cmd);
			}

			m_current_cmd.Reset(); // reset current command for another decoding pass

			boost::asio::async_read(m_socket,
				boost::asio::buffer(&m_current_cmd.orig_message[0], tlv::Header::e_Size),
					boost::bind(&Connection::handle_read_header, shared_from_this(), boost::asio::placeholders::error));
		}
	}

	bool tryHandleCommand (DecodedCommand const & cmd);
	bool handleProfileCommand (DecodedCommand const & cmd);
	bool handleSetupCommand (DecodedCommand const & cmd);

signals:
	void incomingProfilerConnection (profiler::profiler_rvp_t * rvp);
	void incomingProfilerData (profiler::profiler_rvp_t * rvp);
};

} // namespace profiler

