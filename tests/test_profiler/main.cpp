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
#include "mainwindow.h"
#include "blockinfo.h"

#include <QApplication>

using boost::asio::ip::tcp;

int main (int argc, char * argv[])
{
	std::vector<ProfileInfo> profileinfos;
	profileinfos.reserve(8);
	try
	{
		char const * log = 0;
		if (argc < 2)
		{
			printf("Usage: server <port>\n");
			return 1;
		}

		if (argc == 3)
			log = argv[2];

		printf("profiler: server listening...\n");
		boost::asio::io_service io_service;

		tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[1]));
		server_ptr_t tcp_server(new Server(io_service, endpoint, profileinfos));
		io_service.run();
	}
	catch (std::exception & e)
	{
		printf("catserver: Exception in thread: %s\n", e.what());
		fflush(stdout);
	}

    Q_INIT_RESOURCE(resources);

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    MainWindow window(profileinfos);
    window.show();   

    return app.exec();
}
