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

	class CtxPlotConfig : QObject
	{
	public:

		CtxPlotConfig (PlotConfig & cfg, QWidget * parent)
			: m_pcfg(cfg)
			, ui_settingsplot(new Ui::SettingsPlot)
			, m_settingsplot(new QDockWidget(parent))
		{
			ui_settingsplot->setupUi(m_settingsplot);
			setAxisRange(ui_settingsplot->xFromDblSpinBox);
			setAxisRange(ui_settingsplot->yFromDblSpinBox);
			setAxisRange(ui_settingsplot->zFromDblSpinBox);
			m_settingsplot->hide();
		}

		void setAxisRange (QDoubleSpinBox * w)
		{
			w->setMinimum(-1e6);
			w->setMaximum(1e6);
		}

		~CtxPlotConfig ()
		{
			delete ui_settingsplot;
			delete m_settingsplot;
		}

		PlotConfig & m_pcfg;
		Ui::SettingsPlot * ui_settingsplot;
		QDockWidget * m_settingsplot;

		void onShowPlotContextMenu (QPoint const & pos)
		{
			bool const visible = m_settingsplot->isVisible();
			m_settingsplot->setVisible(!visible);
		}

		Ui::SettingsPlot * ui () { return ui_settingsplot; }
	};

}

