
#pragma once
#include <QtGui/qwidget.h>
#include "qwt/qwt_plot.h"
#include "qwt/qwt_plot_canvas.h"
#include "qwt/qwt_plot_layout.h"
#include "curves.h"
#include "config.h"

class QwtPlotCurve;
class QwtPlotMarker;

namespace plot {

	struct BasePlotCurve : public QwtPlotCurve
	{
		BasePlotCurve (QString const & title, CurveConfig & config)
			: QwtPlotCurve(title)
		{
			setColor(config.m_color);
			setRenderHint(QwtPlotItem::RenderAntialiased);
		}
		
		void setColor (QColor const & color)
		{
			QColor c = color;
			c.setAlpha( 150 );
			setPen(c);
			setBrush(c);
		}
	};  

	class BasePlot : public QwtPlot
	{
		Q_OBJECT
	public:

		BasePlot (QWidget * parent, PlotConfig & cfg)
			: QwtPlot(parent)
			, m_curves()
			, m_timer(-1)
		{
			setAutoReplot(false);
			canvas()->setBorderRadius(10);
			plotLayout()->setAlignCanvasToScales(true);

			for (size_t c = 0, ce = cfg.m_ccfg.size(); c < ce; ++c)
			{
				CurveConfig & cc = cfg.m_ccfg[c];
				m_curves.push_back(Curve());
				Curve & curve = m_curves.back();
				curve.m_curve = new BasePlotCurve(cc.m_tag, cc);
				curve.m_data = new Data(cfg.m_history_ln);
				curve.m_curve->attach(this);
				showCurve(curve.m_curve, true);
			}

			for (size_t a = 0, ae = cfg.m_acfg.size(); a < ae; ++a)
			{
				AxisConfig & ac = cfg.m_acfg[a];

				//setAxisTitle(QwtPlot::yLeft, "t [ms]");
				//setAxisScale(QwtPlot::yLeft, 0, 1e6);
				//setAxisTitle(QwtPlot::xBottom, "frame");
				//setAxisScale(QwtPlot::xBottom, 0, m_curve.m_history_ln);
				//setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(cpuStat.upTime()));
				//setAxisLabelRotation(QwtPlot::xBottom, -50.0);
				//setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
			}

			m_timer = startTimer(cfg.m_timer_delay_ms);
		}

		virtual ~BasePlot () { }
		void stopUpdate ()
		{
			killTimer(m_timer);
		}

	protected:
		void timerEvent (QTimerEvent * e)
		{
			update();
		}

		virtual void update ()
		{
			replot();
		}

	protected Q_SLOTS:
		void showCurve (QwtPlotItem * item, bool on)
		{
			item->setVisible(on);
			//if (QwtLegendItem * legendItem = qobject_cast<QwtLegendItem *>( legend()->find(item)))
			//    legendItem->setChecked(on);
			replot();
		}

	protected:
		std::vector<Curve> m_curves;
		int m_timer;
		PlotConfig m_config;
		//std::vector<QwtPlotMarker *> m_markers;
	};
}

