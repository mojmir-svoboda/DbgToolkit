#pragma once
#include <boost/config.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>
#include "profilerconnection.h"

class MainWindow;

namespace profiler {

	struct Server
	{
		Server (boost::asio::io_service & io_service, boost::asio::ip::tcp::endpoint const & endpoint, MainWindow & mw);
		void start_accept ();
		void handle_accept (connection_ptr_t session, boost::system::error_code const & error);

	private:
		boost::asio::io_service & m_io_service;
		boost::asio::ip::tcp::acceptor m_acceptor;
		MainWindow & m_main_window;
	};

	typedef boost::shared_ptr<Server> server_ptr_t;
}

