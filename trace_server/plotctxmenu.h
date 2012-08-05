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

	/*class CtxCurveConfig : QObject
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
	};*/


	class CtxPlotConfig : QObject
	{
	public:

		CtxPlotConfig (PlotConfig & cfg, QWidget * parent)
			: m_parent(parent)
			, m_pcfg(cfg)
			, ui_settingsplot(0)
			, m_settingsplot(0)
		{
		}

		~CtxPlotConfig ()
		{
			delete ui_settingsplot;
			delete m_settingsplot;
		}

		QWidget * m_parent;
		PlotConfig & m_pcfg;
		Ui::SettingsPlot * ui_settingsplot;
		QDockWidget * m_settingsplot;
		//QList<CtxCurveConfig *> m_ccfg;
		//QList<CtxAxisConfig *> m_acfg;

		void onShowPlotContextMenu (QPoint const & pos)
		{
			if (m_settingsplot == 0)
			{
				m_settingsplot = new QDockWidget(m_parent);
				ui_settingsplot = new Ui::SettingsPlot;
				ui_settingsplot->setupUi(m_settingsplot);
			}

			m_settingsplot->show();
			//QPoint globalPos = m_table_view_widget->mapToGlobal(pos);
			//QAction * selectedItem = m_menu_pcfg->exec(QCursor::pos());
		}

		Ui::SettingsPlot * ui () { return ui_settingsplot; }
	};

}

