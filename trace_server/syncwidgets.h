#pragma once
#include <QObject>

enum E_SyncMode {
    e_SyncClientTime,
    e_SyncServerTime,
    e_SyncFrame,
    e_SyncSourceRow,
    e_SyncProxyRow
};

struct SyncWidgets : QObject
{
	SyncWidgets::SyncWidgets (QObject * parent = 0)
		: QObject(parent)
		, m_terminate(false)
	{
		//connect(this, SIGNAL(incomingProfilerConnection(profiler::profiler_rvp_t *)), m_main_window.getServer(), SLOT(incomingProfilerConnection(profiler::profiler_rvp_t *)), Qt::QueuedConnection);
		//printf("+++ connection\n");
	}

	void terminate () { m_terminate = true; }

signals:
	void requestSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source);

public slots:
	void performSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source)
	{
		emit requestSynchronization(mode, sync_group, time, source);
	}

private:
	bool m_terminate;
	Q_OBJECT
};

extern SyncWidgets g_syncWidgets;

inline SyncWidgets & getSyncWidgets () { return g_syncWidgets; }

