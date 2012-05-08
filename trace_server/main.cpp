#include <QtGui/QApplication>
//#include <QtGui/QTableWidget.h>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include "mainwindow.h"
#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
//#	define NOMINMAX
#	include <windows.h>
#endif

struct Application : QApplication
{
	MainWindow * m_main_window;
	Application (int & argc, char *argv[])
		: QApplication(argc, argv)
		, m_main_window(0)
	{}

	void setMainWindow (MainWindow * mw) { m_main_window = mw; }

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

int main(int argc, char *argv[])
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
