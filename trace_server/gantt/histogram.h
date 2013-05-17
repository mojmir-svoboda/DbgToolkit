#pragma once

#include <qpen.h>
#include "qwt/qwt_plot_layout.h"
#include "qwt/qwt_legend.h"
//#include "qwt/qwt_legend_item.h"
#include "qwt/qwt_plot_grid.h"
#include "qwt/qwt_plot_histogram.h"
#include "qwt/qwt_column_symbol.h"
#include "qwt/qwt_series_data.h"

struct Histogram : public QwtPlotHistogram
{
    Histogram (QString const  &, QColor const &);

    void setColor (QColor const &);
    void setValues (unsigned numValues, double const *);
};

