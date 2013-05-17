//#include "profilerblockinfo.h"
//#include "profilerserver.h"

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

	qDebug("profiler: server listening...");
	boost::asio::io_service io_service;

	int const port = 13147; // std::atoi(argv[1])
	tcp::endpoint endpoint(tcp::v4(), port);
	profiler::server_ptr_t tcp_server(new profiler::Server(io_service, endpoint, *m_main_window));

	while (!m_terminate)
	{
		io_service.run();
	}

	qDebug("profiler: server quitting...");
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
		installNativeEventFilter(this);
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
		return false;	
	}
#endif
};

