#pragma once
#include <QtGui/qwidget.h>
#include "qwt/qwt_plot.h"
#include "sessionstate.h"

QT_FORWARD_DECLARE_CLASS(QMainWindow)
class QwtPlotCurve;

namespace stats {

	enum { e_history_ln = 60 };

	struct Curves {
		QwtPlotCurve * m_curve;
		std::vector<double> m_data;
		std::vector<double> m_time_data;
		double m_last;

		Curves ()
			: m_curve(0)
			, m_last(0)
		{
			m_data.reserve(e_history_ln);
			m_time_data.reserve(e_history_ln);
		}
		
		~Curves ()
		{
			if (m_curve) {
				delete m_curve;
				m_curve = 0;
			}
		}
	};

	class StatsPlot : public QwtPlot
	{
		Q_OBJECT
	public:

		enum E_StatsData
		{
			e_ReadBytes,
			e_max_statsdata_enum_value
		};
	
		StatsPlot (QWidget *, SessionState & s);
		~StatsPlot ();
		QwtPlotCurve const * getStatsCurve (E_StatsData const id) const { return m_curves[id].m_curve; }
		void stopUpdate ();

	protected:
		void timerEvent (QTimerEvent * e);

	private Q_SLOTS:
		void showCurve (QwtPlotItem *, bool on);

	private:
		std::vector<Curves> m_curves;
		SessionState & m_state;
	};


	class StatsWindow : public QObject
	{
		Q_OBJECT
	public:
		StatsWindow (QObject * parent, SessionState & state);
		~StatsWindow ();

		void stopUpdate ();
	
	public slots:

	private:
	
		QMainWindow	* m_window;
		StatsPlot * m_plot;
		SessionState & m_state;
	};

}

