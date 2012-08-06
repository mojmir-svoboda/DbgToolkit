#pragma once
#include <QtGui/qwidget.h>
#include <QColorDialog>
#include "qwt/qwt_plot.h"
#include "qwt/qwt_plot_canvas.h"
#include "qwt/qwt_plot_layout.h"
#include "qwt/qwt_symbol.h"
#include "curves.h"
#include "config.h"
#include "plotctxmenu.h"
#include "plottypes.h"

class QwtPlotCurve;
class QwtPlotMarker;

namespace plot {

	struct BasePlotCurve : public QwtPlotCurve
	{
		BasePlotCurve (QString const & title, CurveConfig & config)
			: QwtPlotCurve(title)
		{
			setColor(config.m_color);
			setRenderHint(QwtPlotItem::RenderAntialiased);
		}
		
		void setColor (QColor const & color)
		{
			QColor c = color;
			c.setAlpha( 150 );
			setPen(c);
			setBrush(c);
		}
	};  

	class BasePlot : public QwtPlot
	{
		Q_OBJECT
	public:
		typedef QMap<QString, Curve *> curves_t;

		curves_t::iterator mkCurve (QString const & subtag)
		{
			Curve * curve = new Curve();
			curve->m_curve = new QwtPlotCurve(subtag);
			curve->m_data = new Data(m_config.m_history_ln);
			curve->m_curve->attach(this);
			bool found = false;
			for (size_t i = 0, ie = m_config.m_ccfg.size(); i < ie; ++i)
				if (m_config.m_ccfg[i].m_tag == subtag)
					found = true;
			if (!found)
			{
				CurveConfig cc;
				cc.m_tag = subtag;
				cc.m_label = subtag;
				cc.m_color = allocColor();
				m_config.m_ccfg.push_back(cc);
			}
			// if (enabled)
				showCurve(curve->m_curve, true);
			return m_curves.insert(subtag, curve);
		}

		QColor allocColor () {
			QColor c = m_colors.front();
			m_colors.pop_front();
			return c;
		}

		BasePlot (QWidget * parent, PlotConfig & cfg, QString const & fname)
			: QwtPlot(parent)
			, m_config(cfg)
			, m_config_ui(cfg, this)
			, m_curves()
			, m_timer(-1)
			, m_fname(fname)
		{
			canvas()->setBorderRadius(0);
			setAutoReplot(false);
			plotLayout()->setAlignCanvasToScales(true);

			for (size_t c = 0, ce = cfg.m_ccfg.size(); c < ce; ++c)
			{
				CurveConfig & cc = cfg.m_ccfg[c];
				mkCurve(cc.m_tag); // cc.m_tag is a subtag
			}

			for (size_t a = 0, ae = cfg.m_acfg.size(); a < ae; ++a)
			{
				AxisConfig & ac = cfg.m_acfg[a];
				// ...
			}

			m_timer = startTimer(cfg.m_timer_delay_ms);

			setContextMenuPolicy(Qt::CustomContextMenu);
			connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowPlotContextMenu(QPoint const &)));

			m_colors.push_back(Qt::black);
			m_colors.push_back(Qt::red);
			m_colors.push_back(Qt::darkRed);
			m_colors.push_back(Qt::green);
			m_colors.push_back(Qt::darkGreen);
			m_colors.push_back(Qt::blue);
			m_colors.push_back(Qt::darkBlue);
			m_colors.push_back(Qt::cyan);
			m_colors.push_back(Qt::darkCyan);
			m_colors.push_back(Qt::magenta);
			m_colors.push_back(Qt::darkMagenta);
			m_colors.push_back(Qt::yellow);
			m_colors.push_back(Qt::darkYellow);

			applyConfig(m_config);
		}

		void applyAxis (AxisConfig const & acfg)
		{
			setAxisTitle(acfg.m_axis_pos, acfg.m_label);
			if (!acfg.m_auto_scale)
				setAxisScale(acfg.m_axis_pos, acfg.m_from, acfg.m_to, acfg.m_step);
			else
				setAxisAutoScale(acfg.m_axis_pos, true);

			//setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(cpuStat.upTime()));
			//setAxisLabelRotation(QwtPlot::xBottom, -50.0);
			//setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
		}

		void applyConfig (PlotConfig const & pcfg)
		{
			setTitle(pcfg.m_title);
			for (size_t c = 0, ce = pcfg.m_ccfg.size(); c < ce; ++c)
			{
				CurveConfig const & cc = pcfg.m_ccfg[c];
				Curve * const curve = m_curves[cc.m_tag];
				curve->m_curve->setPen(cc.m_color);
				curve->m_curve->setTitle(cc.m_tag);
				curve->m_curve->setStyle(static_cast<QwtPlotCurve::CurveStyle>(cc.m_style));
				curve->m_curve->setSymbol(new QwtSymbol(static_cast<QwtSymbol::Style>(cc.m_symbol)));
			}

			killTimer(m_timer);
			m_timer = startTimer(pcfg.m_timer_delay_ms);

			applyAxis(pcfg.m_acfg[0]);
			applyAxis(pcfg.m_acfg[1]);

			//for (size_t a = 0, ae = pcfg.m_acfg.size(); a < ae; ++a)
			//	AxisConfig const & ac = pcfg.m_acfg[a];
		}


		virtual ~BasePlot ()
		{
			stopUpdate();
			//@TODO: delete resrcs
			//m_window->hide();
		}
		void stopUpdate ()
		{
			killTimer(m_timer);
		}

		Curve * findCurve (QString const & subtag)
		{
			curves_t::const_iterator it = m_curves.find(subtag);
			if (it == m_curves.end())
			{
				it = mkCurve(subtag);
				//CurveConfig ccfg;
				// load from file?
				// if (!in config)
				//     m_config.m_ccfg.push_back(ccfg);
			}
			return *it;
		}

	protected:
		void timerEvent (QTimerEvent * e)
		{
			update();
		}

		virtual void update ()
		{
			for (curves_t::iterator it = m_curves.begin(), ite = m_curves.end(); it != ite; ++it)
			{
				Curve & curve = **it;
				Data const & data = *curve.m_data;
				size_t n = data.m_data_x.size();
				if (!n)
					continue;

				size_t from = 0;
				int h = m_config.m_history_ln;
				if (m_config.m_auto_scroll)
				{
					from = n > h ? n - h : 0;
				}

				size_t N = n > h ? h : n;
				curve.m_curve->setRawSamples(&data.m_data_x[from], &data.m_data_y[from], N);
			}

			replot();
		}

	public Q_SLOTS:
		void showCurve (QwtPlotItem * item, bool on)
		{
			item->setVisible(on);
			//if (QwtLegendItem * legendItem = qobject_cast<QwtLegendItem *>( legend()->find(item)))
			//    legendItem->setChecked(on);
			replot();
		}

		void onShowPlotContextMenu (QPoint const & pos)
		{
		    qDebug("%s", __FUNCTION__);
			m_config_ui.onShowPlotContextMenu(pos);
			Ui::SettingsPlot * ui = m_config_ui.ui();
			setConfigValues(m_config);
			connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
			connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
			connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(onResetButton()));
			connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(onDefaultButton()));
			connect(ui->curveColorToolButton, SIGNAL(clicked()), this, SLOT(onColorButton()));
			connect(ui->curveComboBox, SIGNAL(activated(int)), this, SLOT(onCurveActivate(int)));

			connect(ui->xAutoScaleCheckBox, SIGNAL(stateChanged(int)), SLOT(onXAutoScaleChanged(int)));
			connect(ui->yAutoScaleCheckBox, SIGNAL(stateChanged(int)), SLOT(onYAutoScaleChanged(int)));
			connect(ui->zAutoScaleCheckBox, SIGNAL(stateChanged(int)), SLOT(onZAutoScaleChanged(int)));
		}

		void setConfigValues (PlotConfig const & pcfg)
		{
		    qDebug("%s", __FUNCTION__);
			Ui::SettingsPlot * ui = m_config_ui.ui();
			ui->curveComboBox->clear();
			for (size_t i = 0, ie = pcfg.m_ccfg.size(); i < ie; ++i)
			{
				CurveConfig const & ccfg = pcfg.m_ccfg[i];
				ui->curveComboBox->addItem(ccfg.m_tag);
			}

			bool const x_auto = pcfg.m_acfg[0].m_auto_scale;
			ui->xAutoScaleCheckBox->setCheckState(x_auto ? Qt::Checked : Qt::Unchecked);
			onXAutoScaleChanged(x_auto ? Qt::Checked : Qt::Unchecked);

			bool const y_auto = pcfg.m_acfg[0].m_auto_scale;
			ui->yAutoScaleCheckBox->setCheckState(y_auto ? Qt::Checked : Qt::Unchecked);
			onYAutoScaleChanged(y_auto ? Qt::Checked : Qt::Unchecked);

			bool const z_auto = pcfg.m_acfg[0].m_auto_scale;
			ui->zAutoScaleCheckBox->setCheckState(z_auto ? Qt::Checked : Qt::Unchecked);
			onZAutoScaleChanged(z_auto ? Qt::Checked : Qt::Unchecked);
			//m_config_ui.ui()->curveComboBox->setCurrentIndex(0);
			for (size_t i = 0; i < enum_to_string_ln_E_PlotSymbol(); ++i)
				ui->symbolComboBox->addItem(QString::fromAscii(enum_to_string_E_PlotSymbol[i]));
			for (size_t i = 0; i < enum_to_string_ln_E_CurveStyle(); ++i)
				ui->styleComboBox->addItem(QString::fromAscii(enum_to_string_E_CurveStyle[i]));

			ui->autoScrollCheckBox->setCheckState(pcfg.m_auto_scroll ? Qt::Checked : Qt::Unchecked);
			ui->updateTimerSpinBox->setValue(pcfg.m_timer_delay_ms);

			onCurveActivate(0);
		}

		void onXAutoScaleChanged (int state)
		{
			Ui::SettingsPlot * ui = m_config_ui.ui();
			bool enabled = state == Qt::Checked ? false : true;
			ui->xFromDblSpinBox->setEnabled(enabled);
			ui->xToDblSpinBox->setEnabled(enabled);
			ui->xStepDblSpinBox->setEnabled(enabled);
			ui->xScaleComboBox->setEnabled(enabled);
		}
		void onYAutoScaleChanged (int state)
		{
			Ui::SettingsPlot * ui = m_config_ui.ui();
			bool enabled = state == Qt::Checked ? false : true;
			ui->yFromDblSpinBox->setEnabled(enabled);
			ui->yToDblSpinBox->setEnabled(enabled);
			ui->yStepDblSpinBox->setEnabled(enabled);
			ui->yScaleComboBox->setEnabled(enabled);
		}
		void onZAutoScaleChanged (int state)
		{
			//Ui::SettingsPlot * ui = m_config_ui.ui();
			//bool enabled = state == Qt::Checked ? false : true;
			//ui->zFromDblSpinBox->setEnabled(enabled);
			//ui->zToDblSpinBox->setEnabled(enabled);
			//ui->zStepDblSpinBox->setEnabled(enabled);
			//ui->zScaleComboBox->setEnabled(enabled);
		}

		void onSaveButton ()
		{
		    qDebug("%s", __FUNCTION__);
			saveConfig(m_config, m_fname);
		}

		void onApplyButton ()
		{
		    qDebug("%s", __FUNCTION__);
			Ui::SettingsPlot * ui = m_config_ui.ui();
			m_config.m_auto_scroll = ui->autoScrollCheckBox->checkState() == Qt::Checked;
			m_config.m_timer_delay_ms = ui->updateTimerSpinBox->value();
			m_config.m_title = ui->titleLineEdit->text();

			int const curveidx = ui->curveComboBox->currentIndex();
			if (curveidx < 0 || curveidx >= m_config.m_ccfg.size())
				return;

			CurveConfig & ccfg = m_config.m_ccfg[curveidx];
			ccfg.m_pen_width = ui->penWidthDblSpinBox->value();
			ccfg.m_style = ui->styleComboBox->currentIndex();
			ccfg.m_symbol = ui->symbolComboBox->currentIndex();

			m_config.m_acfg[0].m_label = ui->xLabelLineEdit->text();
			m_config.m_acfg[0].m_from = ui->xFromDblSpinBox->value();
			m_config.m_acfg[0].m_to = ui->xToDblSpinBox->value();
			m_config.m_acfg[0].m_step = ui->xStepDblSpinBox->value();
			m_config.m_acfg[0].m_scale_type = ui->xScaleComboBox->currentIndex();
			m_config.m_acfg[0].m_auto_scale = ui->xAutoScaleCheckBox->checkState() == Qt::Checked;
			m_config.m_acfg[1].m_label = ui->yLabelLineEdit->text();
			m_config.m_acfg[1].m_from = ui->yFromDblSpinBox->value();
			m_config.m_acfg[1].m_to = ui->yToDblSpinBox->value();
			m_config.m_acfg[1].m_step = ui->yStepDblSpinBox->value();
			m_config.m_acfg[1].m_scale_type = ui->yScaleComboBox->currentIndex();
			m_config.m_acfg[1].m_auto_scale = ui->yAutoScaleCheckBox->checkState() == Qt::Checked;

			//ccfg.m_color = ;

			applyConfig(m_config);
		}

		void onResetButton () { setConfigValues(m_config); }
		void onDefaultButton () { setConfigValues(PlotConfig()); }

		void onCurveActivate (int idx)
		{
			Ui::SettingsPlot * ui = m_config_ui.ui();
			QString const & curvename = ui->curveComboBox->currentText();
			for (size_t i = 0, ie = m_config.m_ccfg.size(); i < ie; ++i)
			{
				CurveConfig const & ccfg = m_config.m_ccfg[i];
				if (ccfg.m_tag == curvename)
				{
					ui->penWidthDblSpinBox->setValue(ccfg.m_pen_width);
					ui->styleComboBox->setCurrentIndex(ccfg.m_style);
					ui->symbolComboBox->setCurrentIndex(ccfg.m_symbol);

					ui->curveColorLabel->setAutoFillBackground(true);
					int const alpha  = 140;
					ui->curveColorLabel->setStyleSheet(tr("QLabel { background-color: rgba(%1, %2, %3, %4); }")
														.arg(ccfg.m_color.red())
														.arg(ccfg.m_color.green())
														.arg(ccfg.m_color.blue())
														.arg(alpha));
					break;
				}
			}
		}

		void onColorButton ()
		{
		    qDebug("%s", __FUNCTION__);
			Ui::SettingsPlot * ui = m_config_ui.ui();
			int const curveidx = ui->curveComboBox->currentIndex();
			QColor const color = QColorDialog::getColor(m_config.m_ccfg[curveidx].m_color);
			if (!color.isValid())
				return;
			m_config.m_ccfg[curveidx].m_color = color;
		}

	protected:
		curves_t m_curves;
		int m_timer;
		PlotConfig & m_config;
		plot::CtxPlotConfig m_config_ui;
		QList<QColor> m_colors;
		QString m_fname;
		//std::vector<QwtPlotMarker *> m_markers;
	};
}

