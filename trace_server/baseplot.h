#pragma once
#include <QtGui/qwidget.h>
#include <QColorDialog>
#include "qwt/qwt_plot.h"
#include "qwt/qwt_plot_canvas.h"
#include "qwt/qwt_plot_layout.h"
#include "qwt/qwt_symbol.h"
#include "curves.h"
#include "config.h"
#include "plotctxmenu.h"
#include "plottypes.h"

class QwtPlotCurve;
class QwtPlotMarker;

namespace plot {

	class BasePlot : public QwtPlot
	{
		Q_OBJECT
	public:
		typedef QMap<QString, Curve *> curves_t;

		BasePlot (QObject * oparent, QWidget * wparent, PlotConfig & cfg, QString const & fname);

		QColor allocColor ();

		void applyAxis (AxisConfig const & acfg);
		void applyConfig (PlotConfig const & pcfg);
		virtual ~BasePlot ();
		void stopUpdate ();

		Curve * findCurve (QString const & subtag);
		curves_t::iterator mkCurve (QString const & subtag);

	protected:
		void timerEvent (QTimerEvent * e);
		virtual void update ();

	public Q_SLOTS:
		void showCurve (QwtPlotItem * item, bool on);

		void onShowPlots ();
		void onHidePlots ();
		void onHidePlotContextMenu ();
		void onShowPlotContextMenu (QPoint const & pos);
		void setConfigValues (PlotConfig const & pcfg);
		void onXAutoScaleChanged (int state);
		void onYAutoScaleChanged (int state);
		void onZAutoScaleChanged (int state);
		void onSaveButton ();
		void onApplyButton ();
		void onResetButton ();
		void onDefaultButton ();
		void onCurveActivate (int idx);
		void onColorButton ();

	protected:
		curves_t m_curves;
		int m_timer;
		PlotConfig & m_config;
		plot::CtxPlotConfig m_config_ui;
		QList<QColor> m_colors;
		QString m_fname;
		//std::vector<QwtPlotMarker *> m_markers;
	};
}

