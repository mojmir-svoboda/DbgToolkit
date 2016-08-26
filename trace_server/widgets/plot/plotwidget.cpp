#include "plotwidget.h"
#include "connection.h"
#include "mainwindow.h"
#include "plottypes.h"
#include <3rd/qwt/qwt_plot_legenditem.h>
#include <3rd/qwt/qwt_legend.h>
#include <3rd/qwt/qwt_legend_label.h>
#include <3rd/qwt/qwt_plot_panner.h>
#include <3rd/qwt/qwt_plot_zoomer.h>
#include <3rd/qwt/qwt_plot_magnifier.h>
#include <3rd/qwt/qwt_picker_machine.h>
#include <3rd/qwt/qwt_plot_marker.h>
#include <constants.h>
#include <serialize/serialize.h>
#include <QTimer>

namespace plot {

	Curve * PlotWidget::mkCurve (QString const & subtag)
	{
		CurveConfig * cc = 0;
		bool found = false;
		for (size_t i = 0, ie = m_config.m_ccfg.size(); i < ie; ++i)
			if (m_config.m_ccfg[i].m_tag == subtag)
			{
				cc = &m_config.m_ccfg[i];
				found = true;
			}
		if (!found)
		{
			CurveConfig cc2;
			cc2.m_tag = subtag;
			cc2.m_label = subtag;
			cc2.m_color = allocColor();
			m_config.m_ccfg.push_back(cc2);
			cc = &m_config.m_ccfg.back();
		}

		QStringList p = path();
		p << subtag;
		Curve * curve = new Curve(*cc, p);
		curve->m_curve = new QwtPlotCurve(subtag);
		curve->applyConfig(*cc);
		curve->m_data = new Data(1024 * 1024);
		curve->m_curve->attach(this);
		// if (enabled)
		m_curves.push_back(curve);
		return curve;
	}

	QColor PlotWidget::allocColor () {
		QColor c = m_colors.front();
		m_colors.pop_front();
		return c;
	}

	PlotWidget::PlotWidget (Connection * conn, PlotConfig const & cfg, QString const & fname, QStringList const & path)
		: QwtPlot(nullptr), DockedWidgetBase(conn->getMainWindow(), path)
		, m_connection(conn)
		, m_config(cfg)
		, m_config_ui(m_config, this)
		, m_curves()
		, m_timer(-1)
		, m_fname(fname)
		, m_legend(nullptr)
		, m_picker(nullptr)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		setAutoReplot(false);
		//canvas()->setBorderRadius(0);
		plotLayout()->setAlignCanvasToScales(true);

		setContentsMargins(QMargins(0, 0, 0, 0));
		setMinimumSize(64,64);

		mkLegend();

		for (size_t c = 0, ce = m_config.m_ccfg.size(); c < ce; ++c)
		{
			CurveConfig & cc = m_config.m_ccfg[c];
			mkCurve(cc.m_tag); // cc.m_tag is a subtag
		}

		for (size_t a = 0, ae = m_config.m_acfg.size(); a < ae; ++a)
		{
			AxisConfig & ac = m_config.m_acfg[a];
			// ...
		}

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		m_colors.push_back(Qt::black);
		m_colors.push_back(Qt::red);
		m_colors.push_back(Qt::darkBlue);
		m_colors.push_back(Qt::magenta);
		m_colors.push_back(Qt::darkGreen);
		m_colors.push_back(Qt::blue);
		m_colors.push_back(Qt::cyan);
		m_colors.push_back(Qt::darkCyan);
		m_colors.push_back(Qt::darkMagenta);
		m_colors.push_back(Qt::green);
		m_colors.push_back(Qt::yellow);
		m_colors.push_back(Qt::darkRed);
		m_colors.push_back(Qt::darkYellow);

		setAutoReplot(true);

		QwtPlotMagnifier * lookglass = new QwtPlotMagnifier(canvas());
		canvas()->setFocusPolicy(Qt::WheelFocus);

		QwtPlotZoomer * zoomer = new QwtPlotZoomer(canvas());
		zoomer->setRubberBandPen(QColor(Qt::black));
		zoomer->setTrackerPen(QColor(Qt::black));
		zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
		zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton, Qt::ShiftModifier);

		m_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
											QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
											canvas());
		m_picker->setStateMachine(new QwtPickerDragPointMachine());
		m_picker->setRubberBandPen(QColor(Qt::green));
		m_picker->setRubberBand(QwtPicker::CrossRubberBand);
		m_picker->setTrackerPen(QColor(Qt::black));
		connect(m_picker, SIGNAL(selected(QRectF const &)), this, SLOT(selected(QRectF const &)));
		connect(m_picker, SIGNAL(selected(QPointF const &)), this, SLOT(selected(QPointF const &)));
		connect(m_picker, SIGNAL(selected(QVector<QPointF> const &)), this, SLOT(selected(QVector<QPointF> const &)));

		connect(&getSyncWidgets(), SIGNAL(requestSynchronization(E_SyncMode, int, unsigned long long, void *)), this, SLOT(performSynchronization(E_SyncMode, int, unsigned long long, void *)));
		connect(this, SIGNAL(requestSynchronization(E_SyncMode, int, unsigned long long, void *)), &getSyncWidgets(), SLOT(performSynchronization(E_SyncMode, int, unsigned long long, void *)));

		QwtPlotPanner * panner = new QwtPlotPanner(canvas());
		panner->setMouseButton(Qt::MidButton);

		setConfigValuesToUI(m_config);
		QTimer::singleShot(0, this, SLOT(onApplyButton()));
		Ui::SettingsPlot * ui = m_config_ui.ui();
		connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		connect(ui->clearDataButton, SIGNAL(clicked()), this, SLOT(onClearAllDataButton()));
		connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(onResetButton()));
		connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(onDefaultButton()));
		connect(ui->curveComboBox, SIGNAL(activated(int)), this, SLOT(onCurveActivate(int)));
		connect(ui->clearCurveDataButton, SIGNAL(clicked()), this, SLOT(onClearCurveDataButton()));
		connect(ui->xAutoScaleCheckBox, SIGNAL(stateChanged(int)), SLOT(onXAutoScaleChanged(int)));
		connect(ui->yAutoScaleCheckBox, SIGNAL(stateChanged(int)), SLOT(onYAutoScaleChanged(int)));
		connect(ui->zAutoScaleCheckBox, SIGNAL(stateChanged(int)), SLOT(onZAutoScaleChanged(int)));

		m_queue.reserve(4096);
	}

	PlotWidget::~PlotWidget ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);

		stopUpdate();

		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowPlotContextMenu(QPoint const &)));
		//disconnect(m_legend, SIGNAL(checked(QVariant const &, bool, int)), SLOT(showItem(QVariant const &, bool)));

		for (Curve * & curve : m_curves)
		{
			curve->m_curve->detach();
			delete curve;
			curve = nullptr;
		}
		m_curves.clear();
	}

	void PlotWidget::onShow ()
	{
		show();
	}

	void PlotWidget::onHide ()
	{
		hide();
	}

	void PlotWidget::applyAxis (AxisConfig const & acfg)
	{
		QwtText title(acfg.m_label);
		QFont f = QFont("Verdana", 7);
		title.setFont(f);
		setAxisTitle(acfg.m_axis_pos, title);
		setAxisFont(acfg.m_axis_pos, f);
		if (!acfg.m_auto_scale)
			setAxisScale(acfg.m_axis_pos, acfg.m_from, acfg.m_to, acfg.m_step);
		else
			setAxisAutoScale(acfg.m_axis_pos, true);
		//setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(cpuStat.upTime()));
		//setAxisLabelRotation(QwtPlot::xBottom, -50.0);
		//setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
	}

	void PlotWidget::mkLegend () 
	{
		m_legend = new QwtLegend(this);
		m_legend->setDefaultItemMode(QwtLegendData::Checkable);
		connect(m_legend, SIGNAL(checked(QVariant const &, bool, int)), SLOT(showItem(QVariant const &, bool)));
		if (m_config.m_legend_show)
		{
			insertLegend(m_legend, static_cast<QwtPlot::LegendPosition>(m_config.m_legend_pos));
			//m_legend->contentsWidget()->setVisible(cfg.m_legend_show);
		}
	}

	void PlotWidget::rmLegend ()
	{
		insertLegend(nullptr); // deletes m_legend
		m_legend = nullptr;
	}


	void PlotWidget::applyConfig (PlotConfig const & pcfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		setTitle(pcfg.m_title);
		if (pcfg.m_legend_show)
		{
			if (!m_legend)
				mkLegend();
			plotLayout()->setLegendPosition(static_cast<QwtPlot::LegendPosition>(pcfg.m_legend_pos));
			//m_legend->contentsWidget()->setVisible(static_cast<QwtPlot::LegendPosition>(pcfg.m_legend_show));
		}
		else
		{
			rmLegend();
		}
		for (size_t c = 0, ce = pcfg.m_ccfg.size(); c < ce; ++c)
		{
			CurveConfig const & cc = pcfg.m_ccfg[c];
			if (Curve * const curve = findOrCreateCurve(cc.m_tag))
			{
				curve->applyConfig(cc);
				showCurve(curve->getCurve(), cc.m_show);
			}
		}

		if (m_timer != -1)
			killTimer(m_timer);
		m_timer = startTimer(pcfg.m_timer_delay_ms);

		applyAxis(pcfg.m_acfg[0]);
		applyAxis(pcfg.m_acfg[1]);
		updateAxes();
	}

	void PlotWidget::stopUpdate ()
	{
		if (m_timer != -1)
			killTimer(m_timer);
	}

	Curve * PlotWidget::findCurve (QString const & subtag)
	{
		for (Curve * c : m_curves)
			if (c->m_config.m_tag == subtag)
				return c;
		return nullptr;
	}
	Curve * PlotWidget::findOrCreateCurve (QString const & subtag)
	{
		if (Curve * c = findCurve(subtag))
			return c;
		return mkCurve(subtag);
	}

	void PlotWidget::timerEvent (QTimerEvent * e)
	{
		update();
	}

	void PlotWidget::update ()
	{
		if (!m_config.m_show)
			return;

		for (curves_t::iterator it = m_curves.begin(), ite = m_curves.end(); it != ite; ++it)
		{
			Curve & curve = **it;
			Data const & data = *curve.m_data;
			size_t n = data.m_data_x.size();
			if (!n)
				continue;

			curve.m_curve->setRawSamples(&data.m_data_x[0], &data.m_data_y[0], static_cast<int>(n));
		}

		replot();
	}

	void PlotWidget::showCurve (QwtPlotItem * item, bool on)
	{
		QVariant const itemInfo = itemToInfo(item);
		if (m_legend)
		{
			QwtLegendLabel * legendLabel = qobject_cast<QwtLegendLabel *>(m_legend->legendWidget(itemInfo));
			if (legendLabel)
				legendLabel->setChecked(true);
		}

		item->setVisible(on);
		replot();
	}

	void PlotWidget::onHideContextMenu ()
	{
		m_config_ui.onHidePlotContextMenu();
	}

	void PlotWidget::onShowContextMenu (QPoint const & pos)
	{
		QRect widgetRect = geometry();
		m_config_ui.onShowPlotContextMenu(QCursor::pos());
		Ui::SettingsPlot * ui = m_config_ui.ui();

		setConfigValuesToUI(m_config);
	}

	void PlotWidget::setConfigValuesToUI (PlotConfig const & pcfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
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
		ui->symbolComboBox->clear();

		E_PlotSymbol shuffled[] = {
			e_NoSymbol,
			e_Cross,
			e_XCross,
			e_HLine,
			e_VLine,
			e_Star1,
			e_Star2,
			e_Diamond,
			e_Triangle,
			e_DTriangle,
			e_UTriangle,
			e_LTriangle,
			e_RTriangle,
			e_Rect,
			e_Hexagon,
			e_Ellipse,
		};
		// static_assert(sizeof == sizeof)
		for (size_t i = 0; i < enum_to_string_ln_E_PlotSymbol(); ++i)
			ui->symbolComboBox->addItem(QString::fromLatin1(enum_to_string_E_PlotSymbol[shuffled[i]]));

		ui->styleComboBox->clear();
		for (size_t i = 0; i < enum_to_string_ln_E_CurveStyle(); ++i)
			ui->styleComboBox->addItem(QString::fromLatin1(enum_to_string_E_CurveStyle[i]));

		m_config.m_title = ui->titleLineEdit->text();
		ui->plotShowCheckBox->setCheckState(pcfg.m_show ? Qt::Checked : Qt::Unchecked);
		ui->autoScrollCheckBox->setCheckState(pcfg.m_auto_scroll ? Qt::Checked : Qt::Unchecked);
		ui->updateTimerSpinBox->setValue(pcfg.m_timer_delay_ms);

		//m_config.m_legend_pos =
		ui->legendShowCheckBox->setChecked(pcfg.m_legend_show);

		if (m_config.m_acfg.size() >= 1)
			ui->xLabelLineEdit->setText(m_config.m_acfg[0].m_label);
		if (m_config.m_acfg.size() >= 2)
			ui->yLabelLineEdit->setText(m_config.m_acfg[1].m_label);
		onCurveActivate(0);
	}

	void PlotWidget::setUIValuesToConfig (PlotConfig & pcfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsPlot * ui = m_config_ui.ui();
		pcfg.m_auto_scroll = ui->autoScrollCheckBox->isChecked();
		pcfg.m_timer_delay_ms = ui->updateTimerSpinBox->value();
		pcfg.m_title = ui->titleLineEdit->text();
		pcfg.m_show = ui->plotShowCheckBox->isChecked();
		pcfg.m_legend_show = ui->legendShowCheckBox->isChecked();
		//m_config.m_legend_pos =

		int const curveidx = ui->curveComboBox->currentIndex();
		if (curveidx >= 0 && curveidx < static_cast<int>(pcfg.m_ccfg.size()))
		{
			CurveConfig & ccfg = pcfg.m_ccfg[curveidx];
			ccfg.m_pen_width = ui->penWidthDblSpinBox->value();
			ccfg.m_style = ui->styleComboBox->currentIndex();
			QString const & sym = ui->symbolComboBox->currentText();
			E_PlotSymbol const sym_enum = enum_from_string_E_PlotSymbol(sym.toStdString().c_str());
			ccfg.m_symbol = sym_enum;
			ccfg.m_symbolsize = ui->symbolSizeSpinBox->value();
			ccfg.m_show = ui->showCurveCheckBox->checkState() == Qt::Checked;
			ccfg.m_color = m_config_ui.m_curve_color->currentColor();
			ccfg.m_symbolcolor = m_config_ui.m_symbol_color->currentColor();

			m_curves[curveidx]->m_config = ccfg;
		}

		pcfg.m_acfg[0].m_label = ui->xLabelLineEdit->text();
		pcfg.m_acfg[0].m_from = ui->xFromDblSpinBox->value();
		pcfg.m_acfg[0].m_to = ui->xToDblSpinBox->value();
		pcfg.m_acfg[0].m_step = ui->xStepDblSpinBox->value();
		pcfg.m_acfg[0].m_scale_type = ui->xScaleComboBox->currentIndex();
		pcfg.m_acfg[0].m_auto_scale = ui->xAutoScaleCheckBox->checkState() == Qt::Checked;
		pcfg.m_acfg[1].m_label = ui->yLabelLineEdit->text();
		pcfg.m_acfg[1].m_from = ui->yFromDblSpinBox->value();
		pcfg.m_acfg[1].m_to = ui->yToDblSpinBox->value();
		pcfg.m_acfg[1].m_step = ui->yStepDblSpinBox->value();
		pcfg.m_acfg[1].m_scale_type = ui->yScaleComboBox->currentIndex();
		pcfg.m_acfg[1].m_auto_scale = ui->yAutoScaleCheckBox->checkState() == Qt::Checked;
	}

	void PlotWidget::onApplyButton ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		setUIValuesToConfig(m_config);
		applyConfig(m_config);
		replot();
	}

	void PlotWidget::onXAutoScaleChanged (int state)
	{
		Ui::SettingsPlot * ui = m_config_ui.ui();
		bool enabled = state == Qt::Checked ? false : true;
		ui->xFromDblSpinBox->setEnabled(enabled);
		ui->xToDblSpinBox->setEnabled(enabled);
		ui->xStepDblSpinBox->setEnabled(enabled);
		ui->xScaleComboBox->setEnabled(enabled);
	}
	void PlotWidget::onYAutoScaleChanged (int state)
	{
		Ui::SettingsPlot * ui = m_config_ui.ui();
		bool enabled = state == Qt::Checked ? false : true;
		ui->yFromDblSpinBox->setEnabled(enabled);
		ui->yToDblSpinBox->setEnabled(enabled);
		ui->yStepDblSpinBox->setEnabled(enabled);
		ui->yScaleComboBox->setEnabled(enabled);
	}
	void PlotWidget::onZAutoScaleChanged (int state) { }

	void PlotWidget::applyConfig ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		applyConfig(m_config);
	}

	QString PlotWidget::getCurrentWidgetPath () const
	{
		QString const appdir = m_connection->getMainWindow()->getAppDir();
		QString const plotpath = appdir + "/" + m_connection->getCurrPreset() + "/" + g_PlotTag + "/" + m_config.m_tag;
		return plotpath;
	}

	void PlotWidget::loadConfig (QString const & preset_dir)
	{
		QString const tag_backup = m_config.m_tag;
		QString const plotpath = getCurrentWidgetPath();
		PlotConfig config;
		bool const loaded = loadConfigTemplate(config, plotpath + "/" + g_PlotFile);
		if (!loaded)
		{
			m_config.clear();
			m_config = config;
			m_config.m_tag = tag_backup; // defaultConfigFor destroys tag
		}
		else
		{
			m_config.clear();
			m_config = config;
		}
		loadAuxConfigs();
	}
	void PlotWidget::loadAuxConfigs ()
	{
		QString const plotpath = getCurrentWidgetPath();
		//m_config.m_find_config.clear();
		//loadConfigTemplate(m_config.m_find_config, plotpath + "/" + g_findTag);
		//filterMgr()->loadConfig(plotpath);
		//colorizerMgr()->loadConfig(plotpath);
	}
	void PlotWidget::saveAuxConfigs ()
	{
		QString const plotpath = getCurrentWidgetPath();
		//saveConfigTemplate(m_config.m_find_config, plotpath + "/" + g_findTag);
		//filterMgr()->saveConfig(plotpath);
		//colorizerMgr()->saveConfig(plotpath);
	}

	void PlotWidget::saveConfig (QString const & path) //@FIXME: unused arg
	{
		QString const plotpath = getCurrentWidgetPath();
		mkPath(plotpath);

		plot::PlotConfig tmp = m_config;
		saveConfigTemplate(tmp, plotpath + "/" + g_PlotFile);
		saveAuxConfigs();
	}

	void PlotWidget::onSaveButton ()
	{
		saveConfig(QString());
	}
	void PlotWidget::onResetButton () {}
	void PlotWidget::onDefaultButton ()
	{
		PlotConfig defaults;
		defaults.partialLoadFrom(m_config);
		setConfigValuesToUI(defaults);
	}

	void PlotWidget::onCurveActivate (int idx)
	{
		Ui::SettingsPlot * ui = m_config_ui.ui();
		QString const & curvename = ui->curveComboBox->currentText();
		for (size_t i = 0, ie = m_config.m_ccfg.size(); i < ie; ++i)
		{
			CurveConfig const & ccfg = m_config.m_ccfg[i];
			if (ccfg.m_tag == curvename)
			{
				ui->showCurveCheckBox->setCheckState(ccfg.m_show ? Qt::Checked : Qt::Unchecked);
				ui->penWidthDblSpinBox->setValue(ccfg.m_pen_width);
				ui->styleComboBox->setCurrentIndex(ccfg.m_style);
				E_PlotSymbol const sym_enum = static_cast<E_PlotSymbol>(ccfg.m_symbol);
				char const * sym_name = enum_to_string_E_PlotSymbol[sym_enum];
				ui->symbolComboBox->setCurrentText(sym_name);
				ui->symbolSizeSpinBox->setValue(ccfg.m_symbolsize);
				m_config_ui.m_curve_color->setCurrentColor(ccfg.m_color);
				m_config_ui.m_symbol_color->setCurrentColor(ccfg.m_color);
				/*ui->curveColorLabel->setAutoFillBackground(true);
				int const alpha  = 140;
				ui->curveColorLabel->setStyleSheet(tr("QLabel { background-color: rgba(%1, %2, %3, %4); }")
													.arg(ccfg.m_color.red())
													.arg(ccfg.m_color.green())
													.arg(ccfg.m_color.blue())
													.arg(alpha));*/
				break;
			}
		}
	}

	void PlotWidget::clearCurveData (QString const & subtag)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		for (curves_t::iterator it = m_curves.begin(), ite = m_curves.end(); it != ite; ++it)
		{
			Curve * curve = *it;
			if (curve && curve->getName() == subtag && curve->m_data)
			{
				curve->m_data->clear();
				curve->m_curve->setRawSamples(0, 0, 0);
			}
		}
		update();
	}

	void PlotWidget::onClearAllDataButton ()
	{
		clearAllData();
	}

	void PlotWidget::clearAllData ()
	{
		for (curves_t::iterator it = m_curves.begin(), ite = m_curves.end(); it != ite; ++it)
		{
			Curve * curve = *it;
			if (curve->m_data)
			{
				curve->m_data->clear();
				curve->m_curve->setRawSamples(0, 0, 0);
			}
		}
		update();
	}

	void PlotWidget::onClearCurveDataButton ()
	{
		Ui::SettingsPlot * ui = m_config_ui.ui();
		clearCurveData(ui->curveComboBox->currentText());
	}

	void PlotWidget::commitCommands (E_ReceiveMode mode)
	{
		for (int i = 0, ie = m_queue.size(); i < ie; ++i)
		{
			DecodedCommand & cmd = m_queue[i];
			if (cmd.present == Command_PR_plot)
				handleDataXYCommand(cmd);
			else if (cmd.present == Command_PR_plotm)
				handleDataXYMarkerCommand(cmd);
		}

		m_queue.clear();
	}

	bool PlotWidget::handleAction (Action * a, E_ActionHandleType sync)
	{
		switch (a->type())
		{
			case e_Close:
			{
				m_connection->destroyDockedWidget(this);
				return true;
			}

			case e_Visibility:
			{
				m_connection->getMainWindow()->applyCachedLayout();
				Q_ASSERT(a->m_args.size() > 0);
				bool const on = a->m_args.at(0).toBool();
				setVisible(on);
				return true;
			}

			case e_Find:
			default:
				return false;
		}
		return false;
	}

	void PlotWidget::setVisible (bool visible)
	{
		m_config.m_show = visible;
		m_dockwidget->setVisible(visible);
		QwtPlot::setVisible(visible);
	}

	void PlotWidget::handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
	{
// 		if (mode == e_RecvSync)
// 		{
// 			if (cmd.m_hdr.cmd == tlv::cmd_plot_xy)
// 				handleDataXYCommand(cmd);
// 			else if (cmd.m_hdr.cmd == tlv::cmd_plot_clear)
// 				handlePlotClearCommand(cmd);
// 		}
// 		else
// 			m_queue.append(cmd);

		m_queue.push_back(cmd);
	}

	bool PlotWidget::handleDataXYCommand (DecodedCommand const & cmd)
	{
		uint64_t const stime = cmd.m_stime;
		uint64_t const ctime = cmd.choice.plot.ctime;

		//tag2type<int_<tag_lvl>> lvl = cmd.choice.plot.lvl;
		//tag2type<int_<tag_ctx>> ctx = cmd.choice.plot.ctx;

		double const x = cmd.choice.plot.x;
		double const y = cmd.choice.plot.y;

		OCTET_STRING const w = cmd.choice.plot.wdgt;
		QString tag = QString::fromLatin1(reinterpret_cast<char const *>(w.buf), w.size);

		int const slash_pos = tag.lastIndexOf(QChar('/'));
		QString subtag = tag;
		tag.chop(tag.size() - slash_pos);
		subtag.remove(0, slash_pos + 1);

		QStringList subpath = path();
		subpath << subtag;

		plot::Curve * curve = findCurve(subtag);
		if (!curve)
		{
			curve = mkCurve(subtag);

			plot::CurveConfig const * ccfg = 0;
			m_config.findCurveConfig(subtag, ccfg); // config is created by mkCurve
			// @TODO: applyCurveConfig

			if (m_config.m_show)
			{
				bool const visible = ccfg ? ccfg->m_show : true;
				showCurve(curve->m_curve, visible);
			}
			else
			{
				bool const visible = ccfg ? ccfg->m_show : false;
				showCurve(curve->m_curve, visible);
			}
		}

		curve->m_data->push_back(stime, ctime, x, y);

		// if (autoscroll && need_to) shift m_from;
		return true;
	}

	bool PlotWidget::handleDataXYMarkerCommand (DecodedCommand const & cmd)
	{
		uint64_t const stime = cmd.m_stime;
		uint64_t const ctime = cmd.choice.plot.ctime;

		//tag2type<int_<tag_lvl>> lvl = cmd.choice.plot.lvl;
		//tag2type<int_<tag_ctx>> ctx = cmd.choice.plot.ctx;

		double const x = cmd.choice.plot.x;
		double const y = cmd.choice.plot.y;

		OCTET_STRING const w = cmd.choice.plot.wdgt;
		QString tag = QString::fromLatin1(reinterpret_cast<char const *>(w.buf), w.size);

		int const slash_pos = tag.lastIndexOf(QChar('/'));
		QString subtag = tag;
		tag.chop(tag.size() - slash_pos);
		subtag.remove(0, slash_pos + 1);

		QStringList subpath = path();
		subpath << subtag;

		plot::Curve * curve = findCurve(subtag);
		if (curve)
		{
			plot::CurveConfig const * ccfg = 0;
			m_config.findCurveConfig(subtag, ccfg); // config is created by mkCurve

			QwtPlotMarker * m = new QwtPlotMarker;
			QwtSymbol * sym = new QwtSymbol(QwtSymbol::Triangle, QBrush(Qt::NoBrush), QPen(ccfg->m_color, 1.0f), QSize(8, 8));
			m->setSymbol(sym);
			m->attach(this);
			m->setValue(x, y);
			m_markers.push_back(m);
		}
		else
		{
			QwtPlotMarker * m = new QwtPlotMarker;
			QwtSymbol * sym = new QwtSymbol(QwtSymbol::DTriangle, QBrush(Qt::NoBrush), QPen(QColor(255, 69, 0), 1.0f), QSize(8, 8));
			m->setSymbol(sym);
			m->setLabel(subtag);
			m->attach(this);
			m->setValue(x, y);
			m_markers.push_back(m);
		}
		return true;
	}


	bool PlotWidget::handlePlotClearCommand (DecodedCommand const & cmd)
	{
// 		QString tag;
// 		for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i) // @TODO: precache
// 		{
// 			if (cmd.m_tvs[i].m_tag == tlv::tag_msg)
// 				tag = cmd.m_tvs[i].m_val;
// 		}
// 		int const slash_pos = tag.lastIndexOf(QChar('/'));
// 		QString subtag = tag;
// 		tag.chop(tag.size() - slash_pos);
// 		subtag.remove(0, slash_pos + 1);
//
// 		qDebug("clear plot: tag='%s' subtag='%s'", tag.toStdString().c_str(), subtag.toStdString().c_str());
//
// 		if (!subtag.isEmpty())
// 			clearCurveData(subtag);
// 		else
// 			clearAllData();
		return true;
	}

	void PlotWidget::selected (QRectF const & r)
	{
		QPointF const pos = r.topLeft();
		float const f = invTransform(QwtPlot::xBottom, pos.x());
		qDebug("selected: %f", f);
		//m_bars->invTransform( QwtPlot::yLeft, pos.y() ),
		//m_bars->invTransform( QwtPlot::yRight, pos.y() )
	}

	void PlotWidget::selected (QVector<QPointF> const & pa)
	{
		qDebug("selected vector of pts");
	}

	void PlotWidget::selected (QPointF const & pa)
	{
		double mindist = 0.0;
		Curve * c = nullptr;
		int minidx = -1;
		double closest = std::numeric_limits<double>::max();
		for (curves_t::iterator it = m_curves.begin(), ite = m_curves.end(); it != ite; ++it)
		{
			(*it)->m_sync_marker->detach();

			double dist = 0.0;
			int const ix = transform(QwtPlot::xBottom, pa.x());
			int const iy = transform(QwtPlot::yLeft, pa.y());
			QPoint pi(ix, iy);
			int const idx = (*it)->m_curve->closestPoint(pi, &dist);
			if (idx != -1)
			{
				if (dist < closest)
				{
					mindist = dist;
					c = *it;
					minidx = idx;
				}
			}
		}

		E_SyncMode const mode = e_SyncServerTime;
		unsigned long long time = 0;
		if (mode == e_SyncServerTime)
		{
			if (c && minidx != -1)
			{
				uint64_t const stm = c->m_data->m_stime[minidx];
				time = stm;
			}
		}
		else
		{
			if (c && minidx != -1)
			{
				uint64_t const ctm = c->m_data->m_ctime[minidx];
				time = ctm;
			}
		}

		emit requestSynchronization(mode, m_config.m_sync_group, time, this);
	}

	void PlotWidget::showItem (QVariant const & itemInfo, bool on)
	{
		QwtPlotItem * plotItem = infoToItem(itemInfo);
		if (plotItem)
			plotItem->setVisible(on);
	}

	void PlotWidget::performSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source)
	{
		if (this == source)
			return;

		qDebug("%s syncgrp=%i time=%i", __FUNCTION__, sync_group, time);

		//if (mode == e_SyncServerTime)
		for (curves_t::iterator it = m_curves.begin(), ite = m_curves.end(); it != ite; ++it)
		{
			uint64_t t = 0;
			Curve * c = *it;
			uint64_t closest2 = std::numeric_limits<uint64_t>::max();
			int closestidx = -1;
			// @TODO @FIXME @NOTE: m_stime and m_ctime should be sorted, binary search should work
			for (size_t i = 0, ie = c->m_data->m_stime.size(); i < ie; ++i)
			{
				uint64_t dist = c->m_data->m_stime[i] - time;
				uint64_t dist2 = dist * dist;
				if (dist2 < closest2)
				{
					closestidx = i;
					closest2 = dist;
				}
			}

			if (c && closestidx != -1)
			{
				double const x = c->m_data->m_data_x[closestidx];
				double const y = c->m_data->m_data_y[closestidx];
				c->m_sync_marker->setValue(x, y);
				c->m_sync_marker->attach(this);
			}
		}
// 		switch (mode)
// 		{
// 			case e_SyncClientTime:
// 			case e_SyncServerTime:
// 			case e_SyncRefSTime:
// 			case e_SyncFrame:
// 			case e_SyncSourceRow:
// 			case e_SyncProxyRow:
// 			default:
// 				break;
// 		}
	}
}

