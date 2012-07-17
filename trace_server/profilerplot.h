#pragma once
#include <QtGui/qwidget.h>
#include "qwt/qwt_plot.h"
#include "curves.h"

class QwtPlotCurve;

namespace profiler {


	class ProfPlot : public QwtPlot
	{
		Q_OBJECT
	public:

		ProfPlot (QWidget *);
		~ProfPlot ();
		void stopUpdate ();

	protected:
		void timerEvent (QTimerEvent * e);

	private Q_SLOTS:
		void showCurve (QwtPlotItem *, bool on);

	private:
		FrameCurve m_curve;
		//SessionState & m_state;
		int m_timer;
	};
}

