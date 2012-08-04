#pragma once
#include <QMenu>
#include <QWidget>
#include <QObject>
#include <QAction>
#include <QWidgetAction>
#include <QLineEdit>
#include <QLabel>
#include "config.h"

namespace plot {

	class CtxCurveConfig : QObject
	{
	public:
		CtxCurveConfig (CurveConfig & config, QMenu * parentmenu)
		{
		}
	};


	class CtxAxisConfig : QObject
	{
	public:
		CtxAxisConfig (AxisConfig & config, QMenu * parentmenu)
		{
		}
	};


	class CtxPlotConfig : QObject
	{
	public:

		CtxPlotConfig (PlotConfig & cfg, QWidget * parent)
			: m_pcfg(cfg)
		{
			m_menu_pcfg = new QMenu("Plot", parent);
			m_tag = new QWidgetAction(parent);
			m_tag->setDefaultWidget(new QLabel(cfg.m_tag));
			m_menu_pcfg->addAction(m_tag);

			m_menu_ccfg = new QMenu("Curve", parent);
			for (int i = 0, ie = cfg.m_ccfg.size(); i < ie; ++i)
				m_ccfg.push_back(new CtxCurveConfig(cfg.m_ccfg[i], m_menu_ccfg));
			m_menu_pcfg->addMenu(m_menu_ccfg);

			m_timer_delay_ms = new QWidgetAction(parent);

			m_tag->setDefaultWidget(new QLineEdit("prdel"));
			m_menu_pcfg->addAction(m_timer_delay_ms);

			//m_toggle_ref = new QAction("Toggle Ref", this);
			//m_hide_prev = new QAction("Hide prev rows", this);
			//m_exclude_fileline = new QAction("Exclude File:Line", this);
		}

		PlotConfig & m_pcfg;
		QMenu * m_menu_pcfg;
		QWidgetAction * m_tag;
		QMenu * m_menu_ccfg;
		QList<CtxCurveConfig *> m_ccfg;
		QList<CtxAxisConfig *> m_acfg;
		QWidgetAction * m_timer_delay_ms;
		QWidgetAction * m_history_ln;

		//int m_timer_delay_ms;
		//int m_history_ln;
		//int m_from;
		// qwt state
		// flags
		bool m_auto_scroll;


		void onShowPlotContextMenu (QPoint const & pos)
		{
			//QPoint globalPos = m_table_view_widget->mapToGlobal(pos);

			QAction * selectedItem = m_menu_pcfg->exec(QCursor::pos()); // @TODO: rather async
			if (selectedItem == m_timer_delay_ms)
			{
				qDebug("jep!");
			}
			else
			{
				qDebug("nip!");
			}
		}
	};

}

