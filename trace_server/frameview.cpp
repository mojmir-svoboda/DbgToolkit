#include "frameview.h"
#include "qwt/qwt_column_symbol.h"

BarPlot::BarPlot () : QwtPlotBarChart()
{
	setLegendMode(QwtPlotBarChart::LegendBarTitles);
    //setLegendIconSize(QSize(10, 14));
}

QwtColumnSymbol * BarPlot::specialSymbol (int index, QPointF const &) const
{
	QwtColumnSymbol * symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
	symbol->setLineWidth(2);
	symbol->setFrameStyle(QwtColumnSymbol::Raised);

	QColor c(Qt::white);
	if (index >= 0 && index < m_colors.size())
		c = m_colors[index];

	symbol->setPalette(c);
	return symbol;
}

QwtText BarPlot::barTitle (int idx) const
{
	QwtText title;
	if (idx >= 0 && idx < m_strvalues.size())
		title = m_strvalues[idx];
	return title;
}


FrameView::FrameView (Connection * oparent, QWidget * wparent, FrameViewConfig & cfg, QString const & fname)
	: m_bars(0)
{

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

	m_bars = new BarPlot();
	m_bars->attach(this);

}

/*void Histogram::setValues (uint numValues, double const * values)
{
    QVector<QwtIntervalSample> samples(numValues);
    for  (unsigned i = 0; i < numValues; ++i)
    {
        QwtInterval interval(double(i), i + 1.0);
        interval.setBorderFlags(QwtInterval::ExcludeMaximum);

        samples[i] = QwtIntervalSample(values[i], interval);
    }

    setData(new QwtIntervalSeriesData(samples ));
}*/

void FrameView::appendFrame (unsigned long long from, unsigned long long to)
{
	qDebug("frame: dt=%f", to - from);
	m_bars->m_values.push_back(to - from);
	m_bars->m_strvalues.push_back(QString("%1").arg(from));
	m_bars->m_colors.push_back(Qt::red);

	m_bars->setSamples(m_bars->m_values);
}


void FrameView::onShow ()
{
	m_bars->show();
}

void FrameView::onHide ()
{
	m_bars->hide();
}


