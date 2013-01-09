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
			, m_show(false)
			, m_unused_b0(true)
			, m_unused_b1(true)
			, m_unused_b2(true)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("label", m_label);
			ar & boost::serialization::make_nvp("pen", m_pen_width);
			ar & boost::serialization::make_nvp("style", m_style);
			ar & boost::serialization::make_nvp("symbol", m_symbol);
			ar & boost::serialization::make_nvp("symcol", m_symbolcolor);
			ar & boost::serialization::make_nvp("symsize", m_symbolsize);
			ar & boost::serialization::make_nvp("color", m_color);
			ar & boost::serialization::make_nvp("show", m_show);
			ar & boost::serialization::make_nvp("flag0", m_unused_b0);
			ar & boost::serialization::make_nvp("flag1", m_unused_b1);
			ar & boost::serialization::make_nvp("flag2", m_unused_b2);
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
			ar & boost::serialization::make_nvp("label", m_label);
			ar & boost::serialization::make_nvp("from", m_from);
			ar & boost::serialization::make_nvp("to", m_to);
			ar & boost::serialization::make_nvp("step", m_step);
			ar & boost::serialization::make_nvp("align", m_alignment);
			ar & boost::serialization::make_nvp("axis", m_axis_pos);
			ar & boost::serialization::make_nvp("rot", m_rotation);
			ar & boost::serialization::make_nvp("scale", m_scale_type);
			ar & boost::serialization::make_nvp("autoscale", m_auto_scale);
			ar & boost::serialization::make_nvp("flag0", m_unused_b0);
			ar & boost::serialization::make_nvp("flag1", m_unused_b1);
			ar & boost::serialization::make_nvp("flag2", m_unused_b2);
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
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("ccfg", m_ccfg);
			ar & boost::serialization::make_nvp("acfg", m_acfg);
			ar & boost::serialization::make_nvp("timer", m_timer_delay_ms);
			ar & boost::serialization::make_nvp("length", m_history_ln);
			//ar & m_from;
			// flags
			ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
			ar & boost::serialization::make_nvp("show", m_show);
			ar & boost::serialization::make_nvp("flag1", m_unused_b1);
			ar & boost::serialization::make_nvp("flag2", m_unused_b2);
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

