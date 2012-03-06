#include <boost/config.hpp>
#include <boost/asio.hpp>
#include <cstdlib>
#include <cstdio>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/asio/socket_base.hpp>
#include "connection.h"
#include "server.h"

using boost::asio::ip::tcp;

int main (int argc, char * argv[])
{
	try
	{
		char const * log = 0;
		if (argc < 2)
		{
			printf("Usage: catserver <port>\n");
			return 1;
		}

		if (argc == 3)
			log = argv[2];

		printf("catserver: starting server...\n");
		boost::asio::io_service io_service;

		tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[1]));
		server_ptr_t tcp_server(new Server(io_service, endpoint));
		io_service.run();
	}
	catch (std::exception & e)
	{
		printf("catserver: Exception in thread: %s\n", e.what());
		fflush(stdout);
	}
	return 0;
}
