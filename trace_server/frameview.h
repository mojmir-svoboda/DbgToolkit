#pragma once
#include <QFrame>
#include "qwt/qwt_plot.h"
#include "qwt/qwt_plot_barchart.h"
#include "frameviewconfig.h"
#include "frameviewctxmenu.h"

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
	Q_OBJECT
public:

	FrameView (Connection * oparent, QWidget * wparent, FrameViewConfig & cfg, QString const & fname);

	void appendFrame (unsigned long long from, unsigned long long to);

public slots:

	void selected (QRectF const & r);
	void selected (QVector<QPointF> const & pa);
	void selected (QPointF const & pos);
	void appended (QPointF const & pos);

	void onShow ();
	void onHide ();
	void onHideContextMenu ();
	void onShowContextMenu (QPoint const & pos);
	void setConfigValuesToUI (FrameViewConfig const & cfg);
	void setUIValuesToConfig (FrameViewConfig & cfg);
	void applyConfig (FrameViewConfig & cfg);
	void onApplyButton ();
	void onSaveButton ();
	void onResetViewButton ();
	void onDefaultButton ();

public:

	BarPlot * m_bars;
	FrameViewConfig & m_config;
	frameview::CtxFrameViewConfig m_config_ui;
};

