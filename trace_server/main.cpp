#include <QtGui/QApplication>
//#include <QtGui/QTableWidget.h>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QThread>
#include "mainwindow.h"
#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
//#	define NOMINMAX
#	include <windows.h>
#endif

#include "profilerblockinfo.h"
#include "profilerserver.h"

class ProfilerAcceptorThread : public QThread
{
	MainWindow * m_main_window;
	bool m_terminate;
	
public:
	void run ();

	ProfilerAcceptorThread () : m_main_window(0), m_terminate(0) { }
	
	void setMainWindow (MainWindow * mw)
	{
		m_main_window = mw;
	}
};

void ProfilerAcceptorThread::run ()
{
	/*if (argc < 2)
	{
		printf("Usage: server <port>\n");
		return;
	}*/
	using boost::asio::ip::tcp;

	qDebug("profiler: server listening...\n");
	boost::asio::io_service io_service;

	int const port = 13147; // std::atoi(argv[1])
	tcp::endpoint endpoint(tcp::v4(), port);
	profiler::server_ptr_t tcp_server(new profiler::Server(io_service, endpoint, *m_main_window));

	while (!m_terminate)
	{
		io_service.run();
	}

	qDebug("profiler: server quitting...\n");
}

struct Application : QApplication
{
	MainWindow * m_main_window;
	ProfilerAcceptorThread m_prof_acceptor_thread;

	Application (int & argc, char *argv[])
		: QApplication(argc, argv)
		, m_main_window(0)
	{}

	~Application ()
	{
		m_prof_acceptor_thread.terminate();
		m_prof_acceptor_thread.wait();
	}

	void setMainWindow (MainWindow * mw)
	{
		m_main_window = mw;
		m_prof_acceptor_thread.setMainWindow(mw);
		m_prof_acceptor_thread.start();
	}

#ifdef WIN32
	bool winEventFilter ( MSG * msg, long * result )
	{
		DWORD const hotkey = VK_SCROLL;
		if (msg->message == WM_HOTKEY)
		{
			qDebug("wineventfilter hotkey");
			//if (GetKeyState(hotkey))
				if (m_main_window)
					m_main_window->onHotkeyShowOrHide();
		}
		return QApplication::winEventFilter(msg, result);
	}
#endif
};

void usage ()
{
	printf("\n(f)Logging server, Copyright (C) 2011 Mojmir Svoboda\n");
	printf("http://developer.berlios.de/projects/flogging\n\n");
	printf("Available options:\n");
	printf("    -q    quit immeadiately if another instance running\n");
	printf("    -n    no visible window at start (can be activated by ScrollLock hotkey)\n");
}

int main (int argc, char *argv[])
{
	bool quit_delay = true;
	bool start_hidden = false;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
        {
            //foo.push_back(argv[i]);
            continue;
        }

        // if there's a flag but no argument following, ignore it
        if (argc == i) continue;
        //char const * arg = argv[i+1];
        switch (argv[i][1])
        {
            case 'q':
				printf("cmd arg: -q, quit immeadiately\n");
				quit_delay = false;
				break;
            case 'n':
				printf("cmd arg: -n, no visible window\n");
				start_hidden = true;
				break;
			case 'h':
				usage();
				return 0;
			default:
				printf("Invalid option, use -h for Help\n");
				return 0;
		}
    }

	Application a(argc, argv);

#ifdef WIN32
	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(0, QObject::tr("Systray"), QObject::tr("I couldn't detect any system tray on this system."));
		return 1;
	}
#endif

	MainWindow w(0, quit_delay);

	if (!start_hidden)
	{
		w.setVisible(true);
		w.show();
	}
	a.setMainWindow(&w);
	if (start_hidden)
		w.onHotkeyShowOrHide();
	return a.exec();
}
