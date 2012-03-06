#pragma once
#include "../tlv_parser/tlv_parser.h"
#include "../tlv_parser/tlv_decoder.h"
#include "../filters/file_filter.hpp"
#include <boost/config.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>


struct DecodedCommand : tlv::StringCommand
{
	enum { e_max_length = 2048 };
	std::vector<char> orig_message;

	DecodedCommand ()
		: StringCommand()
	{
		orig_message.resize(e_max_length);
		tvs.reserve(tlv::tag_max_value);
		Reset();
	}

	void Reset ()
	{
		hdr.Reset();
		tvs.clear();
	}
};


struct Connection : public boost::enable_shared_from_this<Connection>
{
	boost::asio::ip::tcp::socket m_socket;
	DecodedCommand m_current_cmd;
	tlv::TVDecoder m_decoder;

	explicit Connection (boost::asio::io_service & io_service)
		: m_socket(io_service) , m_current_cmd() , m_decoder()
	{ }

	boost::asio::ip::tcp::socket & socket () { return m_socket; }

	void start ()
	{
		printf("+++ new connection\n"); fflush(stdout);
		boost::asio::async_read(m_socket,
			boost::asio::buffer(&m_current_cmd.orig_message[0], tlv::Header::e_Size),
			boost::bind(&Connection::handle_read_header, shared_from_this(), boost::asio::placeholders::error));
	}

	void handle_read_header (boost::system::error_code const & error)
	{
		if (!error)
		{
			m_decoder.decode_header(&m_current_cmd.orig_message[0], tlv::Header::e_Size, m_current_cmd);
			if (m_current_cmd.hdr.cmd == 0 || m_current_cmd.hdr.len == 0)
			{
				return;
			}
	
			// @TODO: len = min(DecodedCommand::e_max_length, m_current_cmd.hdr.len)
			boost::asio::async_read(m_socket,
				boost::asio::buffer(&m_current_cmd.orig_message[tlv::Header::e_Size], m_current_cmd.hdr.len),
					boost::bind(&Connection::handle_read_payload, shared_from_this(), boost::asio::placeholders::error));
		}
		else
		{
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
};

typedef boost::shared_ptr<Connection> connection_ptr_t;
