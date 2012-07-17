#include "profilerplot.h"
#include <QtGui>
#include <QMainWindow>
#include <QColor>
#include "qwt/qwt_plot_curve.h"
#include "qwt/qwt_plot_layout.h"
#include "qwt/qwt_scale_draw.h"
#include "qwt/qwt_scale_widget.h"
#include "qwt/qwt_legend.h"
#include "qwt/qwt_legend_item.h"
#include "qwt/qwt_plot_canvas.h"

namespace profiler {

/*StatsWindow::StatsWindow (QObject * parent, SessionState & state)
	: QObject(parent)
	, m_window(0)
	, m_plot(0)
	, m_state(state)
{
	qDebug("%s", __FUNCTION__);
	m_window = new QMainWindow;
	m_plot = new StatsPlot(0, state);
	m_plot->setTitle("trace traffic");
	m_plot->setContentsMargins(3, 3, 3, 3);
	m_window->setCentralWidget(m_plot);
	m_window->resize(700, 300);
	m_window->show();
}

void StatsWindow::stopUpdate ()
{
	m_plot->stopUpdate();
}
	
StatsWindow::~StatsWindow ()
{
	m_plot->stopUpdate();
	m_window->hide();
	qDebug("%s", __FUNCTION__);
	//delete m_plot;
	//m_plot = 0;
	//delete m_window;
	//m_window = 0;
}
*/

struct TrafficCurve : public QwtPlotCurve
{
    TrafficCurve (QString const & title, QColor const & c)
		: QwtPlotCurve(title)
    {
		setColor(c);
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


ProfPlot::ProfPlot (QWidget * parent)
	: QwtPlot(parent)
	, m_curve(256)
{
	setAutoReplot(false);
	canvas()->setBorderRadius(10);
	plotLayout()->setAlignCanvasToScales(true);

    setAxisTitle(QwtPlot::xBottom, "frame");
	setAxisScale(QwtPlot::xBottom, 0, m_curve.m_history_ln);
    //setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(cpuStat.upTime()));
    //setAxisLabelRotation(QwtPlot::xBottom, -50.0);
    //setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

    setAxisTitle(QwtPlot::yLeft, "t [ms]");
    //setAxisScale(QwtPlot::yLeft, 0, 1e6);
	
	m_curve.m_curve = new TrafficCurve("recv", Qt::blue);
	m_curve.m_curve->attach(this);
	showCurve(m_curve.m_curve, true);

	m_timer = startTimer(500);
}

ProfPlot::~ProfPlot ()
{
	qDebug("%s", __FUNCTION__);
}

void ProfPlot::stopUpdate ()
{
	killTimer(m_timer);
}

void ProfPlot::timerEvent (QTimerEvent *)
{
	//unsigned const diff = m_state.getRecvBytes() - m_curves[e_ReadBytes].m_last;
	//m_curve.m_data.push_back(diff);
	//m_curve.m_last = m_state.getRecvBytes();
	//m_curve.m_time_data.push_back(m_curve.m_time_data.size() / 2.0f);
	
	/*if (m_curve.m_data.size() / 2 > e_history_ln)
	{
		size_t const n = m_curve.m_data.size() - e_history_ln * 2;
		m_curve.m_curve->setRawSamples(&m_curve.m_time_data[n], &m_curve.m_data[n], e_history_ln * 2);
		setAxisScale(QwtPlot::xBottom, n / 2, n / 2 + e_history_ln);
	}
	else
	{
		m_curve.m_curve->setRawSamples(&m_curve.m_time_data[0], &m_curve.m_data[0], m_curve.m_data.size());
	}*/

	replot();
}

void ProfPlot::showCurve (QwtPlotItem * item, bool on)
{
    item->setVisible(on);

    //QwtLegendItem * legendItem = qobject_cast<QwtLegendItem *>( legend()->find(item));
    //if (legendItem)
    //    legendItem->setChecked(on);

    replot();
}

}

