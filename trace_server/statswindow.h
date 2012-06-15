#pragma once
#include <QtGui/qwidget.h>
#include "qwt/qwt_plot.h"

QT_FORWARD_DECLARE_CLASS(QMainWindow)

namespace stats {

	class QwtPlotCurve;

	enum { e_history_ln = 60 };

	class StatsPlot : public QwtPlot
	{
		Q_OBJECT
	public:

		enum E_StatsData
		{
			e_ReadBytes,
			e_max_statsdata_enum_value
		};
	
		StatsPlot (QWidget * = 0);
		QwtPlotCurve const * getStatsCurve (E_StatsData const id) const { return m_data[id].m_curve; }

	protected:
		void timerEvent (QTimerEvent * e);

	private Q_SLOTS:
		void showCurve (QwtPlotItem *, bool on);

	private:
		struct
		{
			QwtPlotCurve * m_curve;
			double m_data[e_history_ln];
		} m_data[e_max_statsdata_enum_value];
	};


	class StatsWindow : public QObject
	{
		Q_OBJECT
	public:
		StatsWindow (QObject * parent = 0);
		~StatsWindow ();
	
	public slots:

	private:
	
		QMainWindow	* m_window;
		StatsPlot * m_plot;
	};

}

