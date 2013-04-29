#include "histogram.h"

Histogram::Histogram (QString const & title, QColor const & symbolColor)
	: QwtPlotHistogram( title )
{
    setStyle(QwtPlotHistogram::Columns);
    setColor(symbolColor);
}

void Histogram::setColor (QColor const & symbolColor)
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

void Histogram::setValues (uint numValues, double const * values)
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

