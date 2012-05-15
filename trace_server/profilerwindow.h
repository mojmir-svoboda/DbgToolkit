#pragma once
#include <QtGui/qwidget.h>
#include "profilerblockinfo.h"

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)

namespace profiler {

	class ProfilerWindow : public QWidget
	{
		Q_OBJECT
	public:
		ProfilerWindow (std::vector<ProfileInfo> & pis, QWidget * parent = 0);
		void populateScene ();

		ProfileInfo const & getProfileInfo (size_t idx = 0) const { return m_profileInfos[idx]; }
		
	private:
		void setupMatrix ();
		
		QGraphicsScene * m_scene;
		std::vector<ProfileInfo> & m_profileInfos;
	};

}

