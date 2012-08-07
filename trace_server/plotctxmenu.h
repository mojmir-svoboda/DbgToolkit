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

	struct CtxPlotConfig : QObject
	{
		PlotConfig & m_pcfg;
		Ui::SettingsPlot * ui_settingsplot;
		QDockWidget * m_settingsplot;

		CtxPlotConfig (PlotConfig & cfg, QWidget * parent)
			: m_pcfg(cfg)
			, ui_settingsplot(new Ui::SettingsPlot)
			, m_settingsplot(new QDockWidget(parent))
		{
			qDebug("%s this=0x%08x", __FUNCTION__, this);
			m_settingsplot->setVisible(false);
			ui_settingsplot->setupUi(m_settingsplot);
			setAxisRange(ui_settingsplot->xFromDblSpinBox);
			setAxisRange(ui_settingsplot->yFromDblSpinBox);
			setAxisRange(ui_settingsplot->zFromDblSpinBox);
		}

		void setAxisRange (QDoubleSpinBox * w)
		{
			w->setMinimum(-1e6);
			w->setMaximum(1e6);
		}

		~CtxPlotConfig ()
		{
			qDebug("%s this=0x%08x", __FUNCTION__, this);

			m_settingsplot->setVisible(false);
			delete ui_settingsplot;
			ui_settingsplot = 0;
			delete m_settingsplot;
			m_settingsplot = 0;
		}

		void onShowPlotContextMenu (QPoint const & pos)
		{
			qDebug("%s", __FUNCTION__);
			bool const visible = m_settingsplot->isVisible();
			m_settingsplot->setVisible(!visible);
		}

		void onHidePlotContextMenu ()
		{
			qDebug("%s", __FUNCTION__);
			m_settingsplot->setVisible(false);
		}

		Ui::SettingsPlot * ui () { return ui_settingsplot; }
	};

}

