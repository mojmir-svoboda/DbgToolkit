#pragma once
#include <QtGui/qwidget.h>
#include "profilerblockinfo.h"
#include "rvps.h"

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)
QT_FORWARD_DECLARE_CLASS(QMainWindow)

namespace profiler {

	class ProfilerWindow : public QObject
	{
		Q_OBJECT
	public:
		ProfilerWindow (QObject * parent = 0, profiler::profiler_rvp_t * rvp = 0);
		void populateScene ();

	private:
		void setupMatrix ();
		
		QGraphicsScene * m_scene;
		QMainWindow	* m_window;
		profiler::profiler_rvp_t * m_rvp;
		ProfileInfo m_profileInfo;
	};

}

