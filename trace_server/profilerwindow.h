#pragma once
#include <QtGui/qwidget.h>
#include <QTreeView>
#include <map>
#include "profilerblockinfo.h"
#include "rvps.h"
#include "profilersessionstate.h"

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
		SessionState & sessionState () { return m_session_state; }
		SessionState const & sessionState () const { return m_session_state; }
	
	public slots:
		void incomingProfilerData (profiler::profiler_rvp_t * rvp);
		void onClickedAtTagTree (QModelIndex idx);

	private:
		void setupMatrix ();
		void registerTag (BlockInfo const & bi);
		void appendToTagTree (std::string const & tagpath);
		void loadState ();
		void onFileColOrExp (QModelIndex const &, bool collapsed);
		void onFileExpanded (QModelIndex const &);
		void onFileCollapsed (QModelIndex const &);
		
		QGraphicsScene * m_scene;
		ProfilerMainWindow	* m_window;
		View * m_view;
		profiler::profiler_rvp_t * m_rvp;
		QTreeView * m_tagWidget;
		SessionState m_session_state;

		static size_t const m_max_unique_colors = 256;
		std::vector<QColor> m_unique_colors;

		typedef std::map<std::string, QColor> colormap_t;
		colormap_t m_tagcolors;
		std::map<unsigned, unsigned> m_max_layers;
	};

}

