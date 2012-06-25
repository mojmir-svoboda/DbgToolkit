#pragma once
#include <QtGui/qwidget.h>
#include <hash_map>
#include <map>
#include "profilerblockinfo.h"
#include "rvps.h"

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)
QT_FORWARD_DECLARE_CLASS(ProfilerMainWindow)

namespace profiler {

	class View;

	class ProfilerWindow : public QObject
	{
		Q_OBJECT
	public:
		ProfilerWindow (QObject * parent = 0, profiler::profiler_rvp_t * rvp = 0);
		~ProfilerWindow ();
	
	public slots:
		void incomingProfilerData (profiler::profiler_rvp_t * rvp);

	private:
		void setupMatrix ();
		
		QGraphicsScene * m_scene;
		ProfilerMainWindow	* m_window;
		View * m_view;
		profiler::profiler_rvp_t * m_rvp;

		static size_t const m_max_unique_colors = 256;
		std::vector<QColor> m_unique_colors;

		typedef std::hash_map<std::string, QColor> colormap_t;
		colormap_t m_tagcolors;
		std::map<unsigned, unsigned> m_max_layers;
	};

}

