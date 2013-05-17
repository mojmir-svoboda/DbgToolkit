#pragma once

#include <qpen.h>
#include "../qwt/qwt_plot_histogram.h"

struct Histogram : public QwtPlotHistogram
{
    Histogram (QString const  &, QColor const &);

    void setColor (QColor const &);
    void setValues (unsigned numValues, double const *);
};

