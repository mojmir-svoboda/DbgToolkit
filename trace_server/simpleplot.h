#pragma once
#include <QtGui/qwidget.h>
#include "baseplot.h"

class QwtPlotCurve;

namespace plot {

	class SimplePlot : public BasePlot
	{
		Q_OBJECT
	public:

		SimplePlot (QWidget *);
		~SimplePlot ();
	private:
	};
}

