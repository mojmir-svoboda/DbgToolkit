#pragma once
#include <QFrame>
#include "qwt/qwt_plot.h"
#include "qwt/qwt_plot_barchart.h"
#include "frameviewconfig.h"

class Connection;

struct BarPlot : QwtPlotBarChart
{
	BarPlot ();

    virtual QwtColumnSymbol * specialSymbol (int index, QPointF const &) const;
	virtual QwtText barTitle (int idx) const;

	QVector<double> m_values;
	std::vector<QString> m_strvalues;
	std::vector<QColor> m_colors;
};

struct FrameView : QwtPlot
{
	FrameView (Connection * oparent, QWidget * wparent, FrameViewConfig & cfg, QString const & fname);

	void appendFrame (unsigned long long from, unsigned long long to);

	void onShow ();
	void onHide ();
	//void onHideContextMenu ();
	//void onShowContextMenu (QPoint const & pos);

public:

	BarPlot * m_bars;
};

