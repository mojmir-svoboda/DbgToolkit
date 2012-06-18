#include "statswindow.h"
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

namespace stats {

StatsWindow::StatsWindow (QObject * parent, SessionState & state)
	: QObject(parent)
	, m_window(0)
	, m_plot(0)
	, m_state(state)
{
	qDebug("%s", __FUNCTION__);
	m_window = new QMainWindow;
	m_plot = new StatsPlot(0, state);
	m_plot->setTitle("traffic");
	m_plot->setContentsMargins(3, 3, 3, 3);
	m_window->setCentralWidget(m_plot);
	m_window->resize(700, 300);
	m_window->show();
}
	
StatsWindow::~StatsWindow ()
{ }


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


StatsPlot::StatsPlot (QWidget * parent, SessionState & state)
	: QwtPlot(parent)
	, m_state(state)
{
	m_curves.resize(e_max_statsdata_enum_value);

	setAutoReplot(false);
	canvas()->setBorderRadius(10);
	plotLayout()->setAlignCanvasToScales(true);

    setAxisTitle(QwtPlot::xBottom, "t");
	setAxisScale(QwtPlot::xBottom, 0, e_history_ln);
    //setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(cpuStat.upTime()));
    //setAxisLabelRotation(QwtPlot::xBottom, -50.0);
    //setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

    setAxisTitle(QwtPlot::yLeft, "recv [B]");
    //setAxisScale(QwtPlot::yLeft, 0, 1e6);
	
	m_curves[e_ReadBytes].m_curve = new TrafficCurve("recv", Qt::green);
	m_curves[e_ReadBytes].m_curve->attach(this);
	showCurve(m_curves[e_ReadBytes].m_curve, true);

	startTimer(500);
}

void StatsPlot::timerEvent (QTimerEvent *)
{
	unsigned const diff = m_state.getRecvBytes() - m_curves[e_ReadBytes].m_last;
	m_curves[e_ReadBytes].m_data.push_back(diff);
	m_curves[e_ReadBytes].m_last = m_state.getRecvBytes();
	m_curves[e_ReadBytes].m_time_data.push_back(m_curves[e_ReadBytes].m_time_data.size() / 2.0f);
	
    //for (size_t c = 0, ce = m_curves.size(); c < ce; ++c)
	{
		size_t const c = e_ReadBytes;
		if (m_curves[c].m_data.size() / 2 > e_history_ln)
		{
			size_t const n = m_curves[c].m_data.size() - e_history_ln * 2;
			m_curves[c].m_curve->setRawSamples(&m_curves[c].m_time_data[n], &m_curves[c].m_data[n], e_history_ln * 2);
			setAxisScale(QwtPlot::xBottom, n / 2, n / 2 + e_history_ln);
		}
		else
		{
			m_curves[c].m_curve->setRawSamples(&m_curves[c].m_time_data[0], &m_curves[c].m_data[0], m_curves[c].m_data.size());
		}
    }

	replot();
}

void StatsPlot::showCurve (QwtPlotItem * item, bool on)
{
    item->setVisible(on);

    //QwtLegendItem * legendItem = qobject_cast<QwtLegendItem *>( legend()->find(item));
    //if (legendItem)
    //    legendItem->setChecked(on);

    replot();
}

}

