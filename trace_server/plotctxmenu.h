#pragma once
#include <QMenu>
#include <QWidget>
#include <QObject>
#include <QAction>
#include <QWidgetAction>
#include <QLineEdit>
#include <QLabel>
#include "config.h"
#include "ui_settingsplot.h"

namespace plot {

	class CtxCurveConfig : QObject
	{
	public:
		CtxCurveConfig (CurveConfig & config, QObject * parentmenu)
		{
		}

		QAction * m_color;
		QWidgetAction * m_width;
		QAction * m_style;
	};


	class CtxAxisConfig : QObject
	{
	public:
		CtxAxisConfig (AxisConfig & config, QObject * parentmenu)
			: m_acfg(config)
		{
		}

		AxisConfig & m_acfg;
		QWidgetAction * m_label;
		QWidgetAction * m_from;
		QWidgetAction * m_to;
		QWidgetAction * m_scale;
		QWidgetAction * m_auto_scale;
	};


	class CtxPlotConfig : QObject
	{
	public:

		CtxPlotConfig (PlotConfig & cfg, QWidget * parent)
			: m_parent(parent)
			, m_pcfg(cfg)
		{
			for (int i = 0, ie = cfg.m_ccfg.size(); i < ie; ++i)
				m_ccfg.push_back(new CtxCurveConfig(cfg.m_ccfg[i], this));

			for (int i = 0, ie = cfg.m_acfg.size(); i < ie; ++i)
				m_acfg.push_back(new CtxAxisConfig(cfg.m_acfg[i], this));
		}

		PlotConfig & m_pcfg;
		QWidget * m_parent;
		Ui::SettingsPlot * ui_settingsplot;
		QDockWidget * m_settingsplot;
		QList<CtxCurveConfig *> m_ccfg;
		QList<CtxAxisConfig *> m_acfg;

		void onShowPlotContextMenu (QPoint const & pos)
		{
			if (ui_settingsplot == 0)
			{
				m_settingsplot = new QDockWidget(m_parent);
				ui_settingsplot = new Ui::SettingsPlot;
				ui_settingsplot->setupUi(m_settingsplot);
			}

			m_settingsplot->show();
			//QPoint globalPos = m_table_view_widget->mapToGlobal(pos);
			//QAction * selectedItem = m_menu_pcfg->exec(QCursor::pos());
		}
	};

}

