#include "frameview.h"
#include "qwt/qwt_column_symbol.h"
#include "qwt/qwt_plot_legenditem.h"
#include "qwt/qwt_legend.h"
#include "qwt/qwt_plot_panner.h"
#include "qwt/qwt_plot_zoomer.h"
#include "qwt/qwt_plot_magnifier.h"
#include "qwt/qwt_picker_machine.h"
#include "qwt/qwt_plot_marker.h"
#include "qwt/qwt_plot_layout.h"
#include "qwt/qwt_color_map.h"
#include "qwt/qwt_interval.h"
#include "qwt/qwt_plot.h"

BarPlot::BarPlot () : QwtPlotBarChart()
{
	setLegendMode(QwtPlotBarChart::LegendBarTitles);
    setLegendIconSize(QSize(10, 14));
}

QwtColumnSymbol * BarPlot::specialSymbol (int index, QPointF const &) const
{
	QwtColumnSymbol * symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
	symbol->setLineWidth(2);
	symbol->setFrameStyle(QwtColumnSymbol::Raised);

	QColor c(Qt::white);
	if (index >= 0 && index < m_colors.size())
		c = m_colors[index];

	symbol->setPalette(c);
	return symbol;
}

QwtText BarPlot::barTitle (int idx) const
{
	QwtText title;
	if (idx >= 0 && idx < m_strvalues.size())
		title = m_strvalues[idx];
	return title;
}


FrameView::FrameView (Connection * oparent, QWidget * wparent, FrameViewConfig & cfg, QString const & fname)
	: m_bars(0)
	, m_config(cfg)
	, m_config_ui(cfg, this)
{

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

	m_bars = new BarPlot();
	m_bars->attach(this);

	plotLayout()->setAlignCanvasToScales(true);
	setContentsMargins(QMargins(0, 0, 0, 0));
	setMinimumSize(64,64);

	setAutoReplot(true);
	//qDebug("%s this=0x%08x", __FUNCTION__, this);

	//insertLegend(new QwtLegend(this), QwtPlot::BottomLegend);
	insertLegend(new QwtLegend(this));

	QwtPlotMagnifier * lookglass = new QwtPlotMagnifier(canvas());
	canvas()->setFocusPolicy(Qt::WheelFocus);

	QwtPlotZoomer * zoomer = new QwtPlotZoomer(canvas());
	zoomer->setRubberBandPen( QColor( Qt::black ) );
	zoomer->setTrackerPen( QColor( Qt::black ) );
	zoomer->setMousePattern( QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier );
	zoomer->setMousePattern( QwtEventPattern::MouseSelect3, Qt::RightButton, Qt::ShiftModifier);

	QwtPlotPicker * picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
										QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
										canvas());
	picker->setStateMachine(new QwtPickerDragPointMachine());
	picker->setRubberBandPen(QColor(Qt::green));
	picker->setRubberBand(QwtPicker::CrossRubberBand);
	picker->setTrackerPen(QColor(Qt::white));

	QwtPlotPanner * panner = new QwtPlotPanner(canvas());
	panner->setMouseButton(Qt::MidButton);

    connect(picker, SIGNAL(selected(QRectF const &) ), this, SLOT(selected(QRectF const &)));
    connect(picker, SIGNAL(selected(QPointF const &) ), this, SLOT(selected(QPointF const &)));
    connect(picker, SIGNAL(appended(QPointF const &) ), this, SLOT(appended(QPointF const &)));
    connect(picker, SIGNAL(selected(QVector<QPointF> const &) ), this, SLOT(selected(QVector<QPointF> const &)));
}

void FrameView::selected (QRectF const & r)
{
	QPointF const pos = r.topLeft();
	float const f = invTransform( QwtPlot::xBottom, pos.x());
	qDebug("selected: %f", f);
	//m_bars->invTransform( QwtPlot::yLeft, pos.y() ),
	//m_bars->invTransform( QwtPlot::yRight, pos.y() )
}

void FrameView::selected (QVector<QPointF> const & pa)
{
	qDebug("selected vector of pts");
}

void FrameView::selected (QPointF const & pa)
{
	qDebug("selected 1 pt");
}
void FrameView::appended (QPointF const & pa)
{
	qDebug("appended 1 pt");
}


/*void Histogram::setValues (uint numValues, double const * values)
{
    QVector<QwtIntervalSample> samples(numValues);
    for  (unsigned i = 0; i < numValues; ++i)
    {
        QwtInterval interval(double(i), i + 1.0);
        interval.setBorderFlags(QwtInterval::ExcludeMaximum);

        samples[i] = QwtIntervalSample(values[i], interval);
    }

    setData(new QwtIntervalSeriesData(samples ));
}*/

void FrameView::appendFrame (unsigned long long from, unsigned long long to)
{
	qDebug("frame: dt=%f", to - from);


	QwtInterval interval(0.0f, 500.0f);
	QwtLinearColorMap colormap(Qt::darkCyan, Qt::red);
	colormap.addColorStop(0.1, Qt::cyan);
	colormap.addColorStop(0.6, Qt::green);
	colormap.addColorStop(0.8, Qt::yellow);
	colormap.addColorStop(0.95, Qt::red);

	QColor const c = colormap.color(interval, to - from);

	m_bars->m_values.push_back(to - from);
	m_bars->m_strvalues.push_back(QString("aa=%1").arg(from));
	m_bars->m_colors.push_back(c);

	m_bars->setSamples(m_bars->m_values);
}


void FrameView::onShow ()
{
	m_bars->show();
}

void FrameView::onHide ()
{
	m_bars->hide();
}

void FrameView::onHideContextMenu ()
{
	m_config_ui.onHideContextMenu();
}

void FrameView::onShowContextMenu (QPoint const & pos)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	QRect widgetRect = geometry();
	m_config_ui.onShowContextMenu(QCursor::pos());
	Ui::SettingsFrameView * ui = m_config_ui.ui();

	setConfigValuesToUI(m_config);

	connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
	connect(ui->clearDataButton, SIGNAL(clicked()), this, SLOT(onClearAllDataButton()));
	connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
	connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(onResetViewButton()));
	//connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(onDefaultButton()));
}

void FrameView::setConfigValuesToUI (FrameViewConfig const & cfg)
{
	//qDebug("%s this=0x%08x", __FUNCTION__, this);
	Ui::SettingsFrameView * ui = m_config_ui.ui();
	
	ui->showCheckBox->blockSignals(true);
	ui->showCheckBox->setCheckState(cfg.m_show ? Qt::Checked : Qt::Unchecked);
	ui->showCheckBox->blockSignals(false);

	double sum = 0.0f;
	for (int i = 0, ie = m_bars->m_values.size(); i < ie; ++i)
	{
		sum += m_bars->m_values[i];
	}
	sum /= m_bars->m_values.size();

	ui->beginSpinBox->setValue(0.0f);
	ui->endSpinBox->setValue(sum);

	ui->color1Button->setCurrentColor(cfg.m_color1);
	ui->color2Button->setCurrentColor(cfg.m_color2);
	ui->color3Button->setCurrentColor(cfg.m_color3);
	ui->color4Button->setCurrentColor(cfg.m_color4);

	ui->color1CheckBox->setChecked(cfg.m_on1);
	ui->color2CheckBox->setChecked(cfg.m_on2);
	ui->color3CheckBox->setChecked(cfg.m_on3);
	ui->color4CheckBox->setChecked(cfg.m_on4);

	ui->color1SpinBox->setValue(cfg.m_val1);
	ui->color2SpinBox->setValue(cfg.m_val2);
	ui->color3SpinBox->setValue(cfg.m_val3);
	ui->color4SpinBox->setValue(cfg.m_val4);
}

void FrameView::setUIValuesToConfig (FrameViewConfig & cfg)
{
	//qDebug("%s this=0x%08x", __FUNCTION__, this);
	Ui::SettingsFrameView * ui = m_config_ui.ui();

	m_config.m_show = ui->showCheckBox->checkState() == Qt::Checked;
}

void FrameView::onApplyButton ()
{
	setUIValuesToConfig(m_config);
	applyConfig(m_config);
}

void FrameView::applyConfig (FrameViewConfig & cfg)
{
	//qDebug("%s this=0x%08x", __FUNCTION__, this);
	Ui::SettingsFrameView * ui = m_config_ui.ui();
}

void FrameView::onSaveButton ()
{
	//m_connection->saveConfigForFrameView(m_config, m_config.m_tag);
}

void FrameView::onResetViewButton ()
{
	setAxisAutoScale(QwtPlot::yLeft, true);
	setAxisAutoScale(QwtPlot::xBottom, true);
}
void FrameView::onDefaultButton ()
{
	FrameViewConfig defaults;
	//defaults.partialLoadFrom(m_config);
	setConfigValuesToUI(defaults);
}



