
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
		typedef QMap<QString, Curve *> curves_t;

		curves_t::iterator mkCurve (QString const & subtag)
		{
			Curve * curve = new Curve();
			curve->m_curve = new QwtPlotCurve(subtag);
			curve->m_data = new Data(m_config.m_history_ln);
			curve->m_curve->attach(this);
			// if (enabled)
				showCurve(curve->m_curve, true);
			return m_curves.insert(subtag, curve);
		}

		BasePlot (QWidget * parent, PlotConfig & cfg)
			: QwtPlot(parent)
			, m_curves()
			, m_timer(-1)
			, m_config(cfg)
		{
			setAutoReplot(false);
			canvas()->setBorderRadius(0);
			plotLayout()->setAlignCanvasToScales(true);
			//setAxisScale(QwtPlot::yLeft, 0, 1);
			//setAxisScale(QwtPlot::xBottom , 0, 1);

			for (size_t c = 0, ce = cfg.m_ccfg.size(); c < ce; ++c)
			{
				CurveConfig & cc = cfg.m_ccfg[c];
				mkCurve(cc.m_tag); // cc.m_tag is a subtag

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

		virtual ~BasePlot ()
		{
			stopUpdate();
			//@TODO: delete resrcs
			//m_window->hide();
		    qDebug("%s", __FUNCTION__);
		}
		void stopUpdate ()
		{
			killTimer(m_timer);
		}

		Curve * findCurve (QString const & subtag)
		{
			curves_t::const_iterator it = m_curves.find(subtag);
			if (it == m_curves.end())
			{
				it = mkCurve(subtag);
				//CurveConfig ccfg;
				// load from file?
				// if (!in config)
				//     m_config.m_ccfg.push_back(ccfg);
			}
			return *it;
		}

	protected:
		void timerEvent (QTimerEvent * e)
		{
			update();
		}

		virtual void update ()
		{
			for (curves_t::iterator it = m_curves.begin(), ite = m_curves.end(); it != ite; ++it)
			{
				Curve & curve = **it;
				Data const & data = *curve.m_data;
				size_t n = data.m_data_x.size();
				if (!n)
					continue;

				size_t from = 0;
				int h = m_config.m_history_ln;
				if (m_config.m_auto_scroll)
				{
					from = n > h ? n - h : 0;
				}

				size_t N = n > h ? h : n;
				curve.m_curve->setRawSamples(&data.m_data_x[from], &data.m_data_y[from], N);
			}

			replot();
		}

	public Q_SLOTS:
		void showCurve (QwtPlotItem * item, bool on)
		{
			item->setVisible(on);
			//if (QwtLegendItem * legendItem = qobject_cast<QwtLegendItem *>( legend()->find(item)))
			//    legendItem->setChecked(on);
			replot();
		}

	protected:
		curves_t m_curves;
		int m_timer;
		PlotConfig m_config;
		//std::vector<QwtPlotMarker *> m_markers;
	};
}

