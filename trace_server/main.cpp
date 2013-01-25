#include <QApplication>
//#include <QtGui/QTableWidget.h>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QThread>
#include <QAbstractNativeEventFilter>
#include "mainwindow.h"
#include <sysfn/os.h>
#include <sysfn/time_query.h>

#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
//#	define NOMINMAX
#	include <windows.h>
#endif

#include "profilerblockinfo.h"
#include "profilerserver.h"

FILE * g_LogRedirect = 0;

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

struct Application : QApplication, public QAbstractNativeEventFilter
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
	virtual bool nativeEventFilter(QByteArray const & eventType, void * message, long * result)
	{
		DWORD const hotkey = VK_SCROLL;
		MSG * msg = static_cast<MSG *>(message);
		if (msg->message == WM_HOTKEY)
		{
			qDebug("wineventfilter hotkey");
			//if (GetKeyState(hotkey))
				if (m_main_window)
					m_main_window->onHotkeyShowOrHide();
			return true;
		}
		return false; //QApplication::nativeEventFilter(eventType, message, result);
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
	printf("    -d    dump mode (csv by default)\n");
}

void qDebugHandler (QtMsgType type, QMessageLogContext const & ctx, QString const & msg)
{
	//@TODO: dump context info
	time_t t = time(NULL);
	switch (type)
	{
		case QtDebugMsg:
			fprintf(g_LogRedirect, "%llu|I|%x|%s\n", t, sys::get_tid(), msg.toLatin1());
			break;
		case QtWarningMsg:
			fprintf(g_LogRedirect, "%llu|W|%x|%s\n", t, sys::get_tid(), msg.toLatin1());
			break;
		case QtCriticalMsg:
			fprintf(g_LogRedirect, "%llu|E|%x|%s\n", t, sys::get_tid(), msg.toLatin1());
			break;
		case QtFatalMsg:
			fprintf(g_LogRedirect, "%llu|F|%x|%s\n", t, sys::get_tid(), msg.toLatin1());
			break;
	}
	fflush(g_LogRedirect);
}

int main (int argc, char * argv[])
{
	QString const log_name = QString("%1.%2").arg(argv[0]).arg("log");
	g_LogRedirect = fopen(log_name.toLatin1(), "a");
	bool quit_delay = true;
	bool start_hidden = false;
	bool dump_mode = false;
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
            case 'd':
				printf("cmd arg: -d, dump mode\n");
				dump_mode = true;
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

	qInstallMessageHandler(qDebugHandler);

	Application a(argc, argv);

#ifdef WIN32
	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(0, QObject::tr("Systray"), QObject::tr("I couldn't detect any system tray on this system."));
		return 1;
	}
#endif

	MainWindow w(0, quit_delay, dump_mode, log_name);

	if (!start_hidden)
	{
		w.setVisible(true);
		w.show();
	}
	a.setMainWindow(&w);
	if (start_hidden)
		w.onHotkeyShowOrHide();
	bool const retval = a.exec();
	qInstallMessageHandler(0);
	fclose(g_LogRedirect);
	return retval;
}
