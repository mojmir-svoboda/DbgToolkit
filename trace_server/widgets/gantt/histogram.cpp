#include "histogram.h"
#include <3rd/qwt/qwt_plot_layout.h>
#include <3rd/qwt/qwt_legend.h>
//#include "qwt/qwt_legend_item.h"
#include <3rd/qwt/qwt_plot_grid.h>
#include <3rd/qwt/qwt_plot_histogram.h>
#include <3rd/qwt/qwt_column_symbol.h>
#include <3rd/qwt/qwt_series_data.h>

Histogram::Histogram (QString const & title, QColor const & symbolColor)
	: QwtPlotHistogram( title )
{
    setStyle(QwtPlotHistogram::Outline);
    setColor(symbolColor);
	//setAxisAutoScale(acfg.m_axis_pos, true);
}

void Histogram::setColor (QColor const & symbolColor)
{
    QColor color = symbolColor;
    color.setAlpha(180);

    setPen(QPen(Qt::black));
    setBrush(QBrush(color));

    QwtColumnSymbol * symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    symbol->setFrameStyle(QwtColumnSymbol::Plain);
    symbol->setLineWidth(1);
    symbol->setPalette(QPalette(color));
    setSymbol(symbol);
}

void Histogram::setValues (uint numValues, double const * values)
{
    QVector<QwtIntervalSample> samples(numValues);
    for ( uint i = 0; i < numValues; i++ )
    {
        QwtInterval interval( double( i ), i + 0.5f );
        interval.setBorderFlags( QwtInterval::ExcludeMaximum );

        samples[i] = QwtIntervalSample( values[i], interval );
    }

    setData( new QwtIntervalSeriesData( samples ) );
}

