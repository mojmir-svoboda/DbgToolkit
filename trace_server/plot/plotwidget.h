#pragma once
#include <qwidget.h>
#include <QColorDialog>
#include "../qwt/qwt_plot.h"
#include "../qwt/qwt_plot_canvas.h"
#include "../qwt/qwt_plot_layout.h"
#include "../qwt/qwt_symbol.h"
#include "curves.h"
#include "plotctxmenu.h"
#include "plottypes.h"

class QwtPlotCurve;
class QwtPlotMarker;

namespace plot {

	class PlotWidget : public QwtPlot
	{
		Q_OBJECT
	public:
		typedef QMap<QString, Curve *> curves_t;

		PlotWidget (QObject * oparent, QWidget * wparent, PlotConfig & cfg, QString const & fname);

		QColor allocColor ();

		void applyAxis (AxisConfig const & acfg);
		void applyConfig (PlotConfig const & pcfg);
		virtual ~PlotWidget ();
		void stopUpdate ();

		Curve * findCurve (QString const & subtag);
		Curve * findOrCreateCurve (QString const & subtag);
		curves_t::iterator mkCurve (QString const & subtag);

		PlotConfig & getConfig () { return m_config; }
		PlotConfig const & getConfig () const { return m_config; }

		void clearAllData ();
		void clearCurveData (QString const & subtag);

	protected:
		void timerEvent (QTimerEvent * e);
		virtual void update ();

	public Q_SLOTS:
		void showCurve (QwtPlotItem * item, bool on);

		void onShow ();
		void onHide ();
		void onHideContextMenu ();
		void onShowContextMenu (QPoint const & pos);
		void setConfigValues (PlotConfig const & pcfg);
		void onXAutoScaleChanged (int state);
		void onYAutoScaleChanged (int state);
		void onZAutoScaleChanged (int state);
		void onSaveButton ();
		void onApplyButton ();
		void onResetButton ();
		void onDefaultButton ();
		void onCurveActivate (int idx);
		void onClearAllDataButton ();
		void onClearCurveDataButton ();

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

