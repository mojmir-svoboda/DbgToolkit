#pragma once
#include <QWidget>
#include <3rd/qwt/qwt_plot.h>
#include <3rd/qwt/qwt_plot_curve.h>
#include <3rd/qwt/qwt_plot_marker.h>
#include "plotconfig.h"

class QwtPlotCurve;
class QwtPlotMarker;

namespace plot {

	struct Data {
		std::vector<uint64_t> m_stime;
		std::vector<uint64_t> m_ctime;
		std::vector<double> m_data_x;
		std::vector<double> m_data_y;

		Data (size_t ln) { reserve(ln); }

		void clear ()
		{
			m_stime.clear();
			m_ctime.clear();
			m_data_x.clear();
			m_data_y.clear();
		}

		void reserve (size_t const n)
		{
			m_stime.reserve(n);
			m_ctime.reserve(n);
			m_data_x.reserve(n);
			m_data_y.reserve(n);
		}

		void push_back (uint64_t stime, uint64_t ctime, double x, double y)
		{
			m_stime.push_back(stime);
			m_ctime.push_back(ctime);
			m_data_x.push_back(x);
			m_data_y.push_back(y);
		}
	};

	struct Curve : ActionAble
	{
		QwtPlotCurve * m_curve;
		QwtPlotMarker * m_sync_marker;
		Data * m_data;
		CurveConfig m_config;

		Curve (CurveConfig const & cfg, QStringList const & path)
			: ActionAble(path)
			, m_curve(nullptr)
			, m_sync_marker(nullptr)
			, m_data(nullptr)
			, m_config(cfg)
		{
			m_sync_marker = new QwtPlotMarker;
			QwtSymbol * sym = new QwtSymbol(QwtSymbol::Diamond, QBrush(Qt::NoBrush), QPen(cfg.m_color, 2.0f), QSize(12, 12));
			m_sync_marker->setSymbol(sym);
		}

		QwtPlotCurve const * getCurve () const { return m_curve; }
		QwtPlotCurve * getCurve () { return m_curve; }

		QString const & getName () { return m_config.m_tag; }

		virtual QWidget * controlWidget () { return 0; }
		virtual bool handleAction (Action * a, E_ActionHandleType sync)
		{
			switch (a->type())
			{
				case e_Visibility:
				{
					Q_ASSERT(a->m_args.size() > 0);
					bool const on = a->m_args.at(0).toBool();
					if (m_curve)
						m_curve->setVisible(on);
					return true;
				}
				default:
					return false;
			}
		}

		~Curve ()
		{
			if (m_curve) {
				m_curve->detach();
				delete m_curve;
				m_curve = nullptr;
			}
			if (m_sync_marker) {
				delete m_sync_marker;
				m_sync_marker = nullptr;
			}
			if (m_data) {
				delete m_data;
				m_data = nullptr;
			}
		}

		void applyConfig (CurveConfig const & cc)
		{
			m_config = cc;
			m_curve->setPen(QPen(cc.m_color));
			//curve->m_curve->setBrush(QBrush(cc.m_color));
			m_curve->setTitle(cc.m_tag);
			m_curve->setStyle(static_cast<QwtPlotCurve::CurveStyle>(cc.m_style - 1));
			QwtSymbol * symbol = new QwtSymbol(static_cast<QwtSymbol::Style>(cc.m_symbol - 1));
			symbol->setSize(cc.m_symbolsize);
			symbol->setPen(QPen(cc.m_symbolcolor));
			m_curve->setSymbol(symbol);
			m_curve->setBaseline(cc.m_pen_width);
			m_curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
			m_curve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
		}
	};
}

