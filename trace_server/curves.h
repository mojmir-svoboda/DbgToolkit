#pragma once
#include <QtGui/qwidget.h>
#include "qwt/qwt_plot.h"

class QwtPlotCurve;

namespace profiler {

	struct FrameCurve {
		QwtPlotCurve * m_curve;
		std::vector<double> m_data;
		std::vector<double> m_time_data;
		double m_last;
		size_t m_history_ln;

		FrameCurve (size_t ln)
			: m_curve(0)
			, m_last(0)
			, m_history_ln(ln)
		{
			m_data.reserve(m_history_ln);
			m_time_data.reserve(m_history_ln);
		}
		
		~FrameCurve ()
		{
			if (m_curve) {
				delete m_curve;
				m_curve = 0;
			}
			m_data.clear();
			m_time_data.clear();
		}
	};
}

