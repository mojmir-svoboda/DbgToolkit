#pragma once
#include <QMenu>
#include <QWidget>
#include <QObject>
#include <QAction>
#include <QWidgetAction>
#include <QLineEdit>
#include <QLabel>
#include "ui_settingsplot.h"
#include "../qtsln/qtcolorpicker/qtcolorpicker.h"

namespace plot {

	struct CtxPlotConfig : QObject
	{
		PlotConfig & m_pcfg;
		Ui::SettingsPlot * m_ui;
		QDockWidget * m_settingsplot;
		QtColorPicker * m_curve_color;
		QtColorPicker * m_symbol_color;

		CtxPlotConfig (PlotConfig & cfg, QWidget * parent)
			: m_pcfg(cfg)
			, m_ui(new Ui::SettingsPlot)
			, m_settingsplot(new QDockWidget(parent))
			, m_curve_color(new QtColorPicker(parent))
			, m_symbol_color(new QtColorPicker(parent))
		{
			m_settingsplot->setVisible(false);
			m_ui->setupUi(m_settingsplot);
			m_ui->curveColorLayout->addWidget(m_curve_color);
			m_ui->symbolColorLayout->addWidget(m_symbol_color);
			m_curve_color->setStandardColors();
			m_symbol_color->setStandardColors();
			setAxisRange(m_ui->xFromDblSpinBox);
			setAxisRange(m_ui->yFromDblSpinBox);
			setAxisRange(m_ui->zFromDblSpinBox);
		}

		void setAxisRange (QDoubleSpinBox * w)
		{
			w->setMinimum(-1e6);
			w->setMaximum(1e6);
		}

		~CtxPlotConfig ()
		{
			m_settingsplot->setVisible(false);
			delete m_ui;
			m_ui = 0;
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

		Ui::SettingsPlot * ui () { return m_ui; }
	};

}

