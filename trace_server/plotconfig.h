#pragma once
#include "types.h"

namespace plot {

	struct CurveConfig
	{
		QString m_tag;
		QString m_label;
		float m_pen_width;
		int m_style;
		int m_symbol;
		QColor m_color;
		QColor m_symbolcolor;
		int m_symbolsize;
		bool m_show;
		bool m_unused_b0;
		bool m_unused_b1;
		bool m_unused_b2;

		CurveConfig ()
			: m_pen_width(0.0f)
			, m_style(1)
			, m_symbol(0)
			, m_color(Qt::red)
			, m_symbolcolor(Qt::red)
			, m_symbolsize(6)
			, m_show(true)
			, m_unused_b0(true)
			, m_unused_b1(true)
			, m_unused_b2(true)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & m_tag;
			ar & m_label;
			ar & m_pen_width;
			ar & m_style;
			ar & m_symbol;
			ar & m_symbolcolor;
			ar & m_symbolsize;
			ar & m_color;
			ar & m_show;
			ar & m_unused_b0;
			ar & m_unused_b1;
			ar & m_unused_b2;
		}
	};

	struct AxisConfig
	{
		QString m_label;
		double m_from;
		double m_to;
		double m_step;
		int m_scale_type;
		int m_axis_pos;
		int m_alignment;
		double m_rotation;
		bool m_auto_scale;
		bool m_unused_b0;
		bool m_unused_b1;
		bool m_unused_b2;

		AxisConfig ()
			: m_label()
			, m_from(0.0f)
			, m_to(1.0f)
			, m_step(0.0f)
			, m_scale_type(0)
			, m_axis_pos(0)
			, m_alignment(0)
			, m_rotation(0.0f)
			, m_auto_scale(true)
			, m_unused_b0(true)
			, m_unused_b1(true)
			, m_unused_b2(true)
		{ }
			

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & m_label;
			ar & m_from;
			ar & m_to;
			ar & m_step;
			ar & m_alignment;
			ar & m_axis_pos;
			ar & m_rotation;
			ar & m_scale_type;
			ar & m_auto_scale;
			ar & m_unused_b0;
			ar & m_unused_b1;
			ar & m_unused_b2;
		}
	};

	struct PlotConfig
	{
		QString m_tag;
		QString m_title;
		QList<CurveConfig> m_ccfg;
		QList<AxisConfig> m_acfg;

		int m_timer_delay_ms;
		int m_history_ln;
		int m_from;
		// qwt state
		// flags
		bool m_auto_scroll;
		bool m_show;
		bool m_unused_b1;
		bool m_unused_b2;

		PlotConfig ()
			: m_tag()
			, m_timer_delay_ms(50)
			, m_history_ln(256)
			, m_from(0)
			, m_auto_scroll(true)
			, m_show(true)
			, m_unused_b1(false)
			, m_unused_b2(false)
		{
			m_acfg.push_back(AxisConfig());
			m_acfg.back().m_axis_pos = 2; //QwtPlot::xBottom;
				//yLeft, yRight, xBottom, xTop,
			m_acfg.push_back(AxisConfig());
			m_acfg.back().m_axis_pos = 0; //QwtPlot::yLeft;
		}

		PlotConfig (QString const & tag)
			: m_tag(tag)
			, m_timer_delay_ms(50)
			, m_history_ln(256)
			, m_from(0)
			, m_auto_scroll(true)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & m_tag;
			ar & m_ccfg;
			ar & m_acfg;
			ar & m_timer_delay_ms;
			ar & m_history_ln;
			//ar & m_from;
			// flags
			ar & m_auto_scroll;
			ar & m_show;
			ar & m_unused_b1;
			ar & m_unused_b2;
		}

		void partialLoadFrom (PlotConfig const & rhs)
		{
			m_tag = rhs.m_tag;
			m_ccfg = rhs.m_ccfg;
		}

		bool findCurveConfig (QString const & tag, CurveConfig const * & ccfg)
		{
			for (int i = 0, ie = m_ccfg.size(); i < ie; ++i)
				if (m_ccfg.at(i).m_tag == tag)
				{
					ccfg = &m_ccfg.at(i);
					return true;
				}
			return false;
		}
	};

	bool loadConfig (PlotConfig & config, QString const & fname);
	bool saveConfig (PlotConfig const & config, QString const & fname);
}

