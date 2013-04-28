#pragma once

#include <qpen.h>
#include "qwt/qwt_plot_layout.h"
#include "qwt/qwt_legend.h"
//#include "qwt/qwt_legend_item.h"
#include "qwt/qwt_plot_grid.h"
#include "qwt/qwt_plot_histogram.h"
#include "qwt/qwt_column_symbol.h"
#include "qwt/qwt_series_data.h"

class Histogram : public QwtPlotHistogram
{
public:
    Histogram (QString const  &, QColor const &);

    void setColor (QColor const &);
    void setValues (unsigned numValues, double const *);
};

inline Histogram::Histogram (QString const & title, QColor const & symbolColor)
	: QwtPlotHistogram( title )
{
    setStyle(QwtPlotHistogram::Columns);
    setColor(symbolColor);
}

inline void Histogram::setColor (QColor const & symbolColor)
{
    QColor color = symbolColor;
    color.setAlpha(180);

    setPen(QPen(Qt::black));
    setBrush(QBrush(color));

    QwtColumnSymbol * symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    symbol->setFrameStyle(QwtColumnSymbol::Raised);
    symbol->setLineWidth(2);
    symbol->setPalette(QPalette(color));
    setSymbol(symbol);
}

inline void Histogram::setValues (uint numValues, double const * values)
{
    QVector<QwtIntervalSample> samples(numValues);
    for ( uint i = 0; i < numValues; i++ )
    {
        QwtInterval interval( double( i ), i + 1.0 );
        interval.setBorderFlags( QwtInterval::ExcludeMaximum );

        samples[i] = QwtIntervalSample( values[i], interval );
    }

    setData( new QwtIntervalSeriesData( samples ) );
}

