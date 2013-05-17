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
#include "qtsln/qtcolorpicker/qtcolorpicker.h"

namespace plot {

	struct CtxPlotConfig : QObject
	{
		PlotConfig & m_pcfg;
		Ui::SettingsPlot * ui_settingsplot;
		QDockWidget * m_settingsplot;
		QtColorPicker * m_curve_color;
		QtColorPicker * m_symbol_color;

		CtxPlotConfig (PlotConfig & cfg, QWidget * parent)
			: m_pcfg(cfg)
			, ui_settingsplot(new Ui::SettingsPlot)
			, m_settingsplot(new QDockWidget(parent))
			, m_curve_color(new QtColorPicker(parent))
			, m_symbol_color(new QtColorPicker(parent))
		{
			m_settingsplot->setVisible(false);
			ui_settingsplot->setupUi(m_settingsplot);
			ui_settingsplot->curveColorLayout->addWidget(m_curve_color);
			ui_settingsplot->symbolColorLayout->addWidget(m_symbol_color);
			m_curve_color->setStandardColors();
			m_symbol_color->setStandardColors();
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
			m_settingsplot->setVisible(false);
			delete ui_settingsplot;
			ui_settingsplot = 0;
			delete m_settingsplot;
			m_settingsplot = 0;
		}

		void onShowPlotContextMenu (QPoint const & pos)
		{
			bool const visible = m_settingsplot->isVisible();
			m_settingsplot->setVisible(!visible);

			if (m_settingsplot->isVisible())
				m_settingsplot->move(pos);
		}

		void onHidePlotContextMenu ()
		{
			m_settingsplot->setVisible(false);
		}

		Ui::SettingsPlot * ui () { return ui_settingsplot; }
	};

}

