#include "pickableplot.h"
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

struct ColoredCurve : public QwtPlotCurve
{
    ColoredCurve (QString const & title, QColor const & c)
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


PickablePlot::PickablePlot (QWidget * parent)
	: QwtPlot(parent)
	, m_curve(512)
{
	setAutoReplot(false);
	//canvas()->setBorderRadius(10);
	plotLayout()->setAlignCanvasToScales(true);

    setAxisTitle(QwtPlot::xBottom, "frame");
	setAxisScale(QwtPlot::xBottom, 0, m_curve.m_history_ln);
    //setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(cpuStat.upTime()));
    //setAxisLabelRotation(QwtPlot::xBottom, -50.0);
    //setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

    setAxisTitle(QwtPlot::yLeft, "t [ms]");
    //setAxisScale(QwtPlot::yLeft, 0, 1e6);
	
	m_curve.m_curve = new ColoredCurve("recv", Qt::blue);
	m_curve.m_curve->attach(this);
	showCurve(m_curve.m_curve, true);

	m_timer = startTimer(500);


/*



    d_zoomer[0] = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft,
        d_plot->canvas() );
    d_zoomer[0]->setRubberBand( QwtPicker::RectRubberBand );
    d_zoomer[0]->setRubberBandPen( QColor( Qt::green ) );
    d_zoomer[0]->setTrackerMode( QwtPicker::ActiveOnly );
    d_zoomer[0]->setTrackerPen( QColor( Qt::white ) );

    d_zoomer[1] = new Zoomer( QwtPlot::xTop, QwtPlot::yRight,
         d_plot->canvas() );


    d_panner = new QwtPlotPanner( d_plot->canvas() );
    d_panner->setMouseButton( Qt::MidButton );



    d_picker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
        QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
        d_plot->canvas() );
    d_picker->setStateMachine( new QwtPickerDragPointMachine() );
    d_picker->setRubberBandPen( QColor( Qt::green ) );
    d_picker->setRubberBand( QwtPicker::CrossRubberBand );
    d_picker->setTrackerPen( QColor( Qt::white ) );
*/

}

/*

    connect( cntDamp, SIGNAL( valueChanged( double ) ),
        d_plot, SLOT( setDamp( double ) ) );

    connect( d_picker, SIGNAL( moved( const QPoint & ) ),
        SLOT( moved( const QPoint & ) ) );
    connect( d_picker, SIGNAL( selected( const QPolygon & ) ),
        SLOT( selected( const QPolygon & ) ) );


void MainWindow::enableZoomMode( bool on )
{
    d_panner->setEnabled( on );

    d_zoomer[0]->setEnabled( on );
    d_zoomer[0]->zoom( 0 );
    
    d_zoomer[1]->setEnabled( on );
    d_zoomer[1]->zoom( 0 ); 

    d_picker->setEnabled( !on );
    
    showInfo();
} 

void MainWindow::moved( const QPoint &pos )
{
    QString info;
    info.sprintf( "Freq=%g, Ampl=%g, Phase=%g",
        d_plot->invTransform( QwtPlot::xBottom, pos.x() ),
        d_plot->invTransform( QwtPlot::yLeft, pos.y() ),
        d_plot->invTransform( QwtPlot::yRight, pos.y() )
    );
    showInfo( info );
}

void MainWindow::selected( const QPolygon & ) { showInfo(); }








  
*/

PickablePlot::~PickablePlot ()
{
	qDebug("%s", __FUNCTION__);
}

void PickablePlot::stopUpdate ()
{
	killTimer(m_timer);
}

void PickablePlot::timerEvent (QTimerEvent *)
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

void PickablePlot::showCurve (QwtPlotItem * item, bool on)
{
    item->setVisible(on);

    //QwtLegendItem * legendItem = qobject_cast<QwtLegendItem *>( legend()->find(item));
    //if (legendItem)
    //    legendItem->setChecked(on);

    replot();
}

}

