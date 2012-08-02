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

namespace plot {

PickablePlot::PickablePlot (QWidget * parent)
	: BasePlot(parent, PlotConfig())
{
	//m_curve.m_curve = new ColoredCurve("pickable plot", Qt::red);
	//m_curve.m_curve->attach(this);
	//showCurve(m_curve.m_curve, true);
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

PickablePlot::~PickablePlot () { qDebug("%s", __FUNCTION__); }

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

}

