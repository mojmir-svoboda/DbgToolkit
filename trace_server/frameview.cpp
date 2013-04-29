#include "frameview.h"

FrameView::FrameView (Connection * oparent, QWidget * wparent, FrameViewConfig & cfg, QString const & fname)
	: QFrame(wparent), QwtPlotBarChart()
{

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
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
	m_values.push_back(to - from);
	m_strvalues.push_back(QString("%1").arg(from));
	m_colors.push_back(Qt::red);

	setSamples(m_values);
}


void FrameView::onShow ()
{
	QFrame::show();
	QwtPlotBarChart::show();
}

void FrameView::onHide ()
{
	QwtPlotBarChart::hide();
}


