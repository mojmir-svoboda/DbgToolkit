#include "frameview.h"
#include "../qwt/qwt_column_symbol.h"
#include "../qwt/qwt_plot_legenditem.h"
#include "../qwt/qwt_legend.h"
#include "../qwt/qwt_plot_panner.h"
#include "../qwt/qwt_plot_zoomer.h"
#include "../qwt/qwt_plot_magnifier.h"
#include "../qwt/qwt_picker_machine.h"
#include "../qwt/qwt_plot_marker.h"
#include "../qwt/qwt_plot_layout.h"
#include "../qwt/qwt_color_map.h"
#include "../qwt/qwt_interval.h"
#include "../qwt/qwt_plot.h"
#include "../qwt/qwt_scale_draw.h"
#include "../syncwidgets.h"

struct FrameScaleDraw : public QwtScaleDraw
{
public:
    FrameScaleDraw (Qt::Orientation orientation, std::vector<QString> const & labels)
		: m_labels( labels )
    {
        setTickLength(QwtScaleDiv::MinorTick, 0);
        setTickLength(QwtScaleDiv::MediumTick, 0);
        setTickLength(QwtScaleDiv::MajorTick, 2);

        enableComponent(QwtScaleDraw::Backbone, false);

        if (orientation == Qt::Vertical)
            setLabelRotation( -90.0 );
        else
            setLabelRotation( -90.0 );

        setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }

    virtual QwtText label (double value) const
    {
        QwtText lbl;
        int const index = qRound(value);
		//@TODO: zde je v examplu chyba
        if (index >= 0 && index < m_labels.size())
            lbl = m_labels[index];
        return lbl;
    }

private:
	std::vector<QString> const & m_labels;
};


BarPlot::BarPlot () : QwtPlotBarChart()
{
}

QwtColumnSymbol * BarPlot::specialSymbol (int index, QPointF const &) const
{
	QwtColumnSymbol * symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
	symbol->setLineWidth(1);
	symbol->setFrameStyle(QwtColumnSymbol::Plain);

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

	struct XZoomer : QwtPlotZoomer
	{
		XZoomer (QWidget * canvas) : QwtPlotZoomer(canvas) { }
		virtual void zoom (QRectF const & rect)
		{
			QRectF newRect;
			QRectF const & baseRect = zoomBase();
			newRect.setCoords(rect.left(), baseRect.top(), rect.right(), baseRect.bottom());
			QwtPlotZoomer::zoom(newRect);
		}
	};

FrameView::FrameView (Connection * oparent, QWidget * wparent, FrameViewConfig & cfg, QString const & fname, QStringList const & path)
	: ActionAble(path)
	, m_bars(0)
	, m_config(cfg)
	, m_config_ui(cfg, this)
{

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

	m_bars = new BarPlot();
	m_bars->attach(this);

	plotLayout()->setAlignCanvasToScales(true);
	plotLayout()->setCanvasMargin(0);
	setContentsMargins(QMargins(0, 0, 0, 0));
	setMinimumSize(64,64);

	setAutoReplot(true);
	//qDebug("%s this=0x%08x", __FUNCTION__, this);

	QwtPlotMagnifier * lookglass = new QwtPlotMagnifier(canvas());
	canvas()->setFocusPolicy(Qt::WheelFocus);
	lookglass->setAxisEnabled(QwtPlot::yLeft, false);

	XZoomer * zoomer = new XZoomer(canvas());
	zoomer->setRubberBandPen( QColor( Qt::black ) );
	zoomer->setTrackerPen( QColor( Qt::black ) );
	zoomer->setMousePattern( QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier );
	zoomer->setMousePattern( QwtEventPattern::MouseSelect3, Qt::RightButton, Qt::ShiftModifier);

	QwtPlotPicker * picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
										QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
										canvas());
	//picker->setStateMachine(new QwtPickerDragPointMachine());
	picker->setStateMachine(new QwtPickerClickPointMachine());
	picker->setRubberBandPen(QColor(Qt::green));
	picker->setRubberBand(QwtPicker::CrossRubberBand);
	picker->setTrackerPen(QColor(Qt::white));

	QwtPlotPanner * panner = new QwtPlotPanner(canvas());
	panner->setAxisEnabled(QwtPlot::yLeft, false);
	panner->setMouseButton(Qt::MidButton);

    setAxisMaxMinor(QwtPlot::xBottom, 3);
    setAxisScaleDraw(QwtPlot::xBottom, new FrameScaleDraw(Qt::Horizontal, m_bars->m_strvalues));

	setAxisAutoScale(QwtPlot::yLeft, true);

    connect(picker, SIGNAL(selected(QRectF const &) ), this, SLOT(selected(QRectF const &)));
    connect(picker, SIGNAL(selected(QPointF const &) ), this, SLOT(selected(QPointF const &)));
    connect(picker, SIGNAL(appended(QPointF const &) ), this, SLOT(appended(QPointF const &)));
    connect(picker, SIGNAL(selected(QVector<QPointF> const &) ), this, SLOT(selected(QVector<QPointF> const &)));

    connect(&getSyncWidgets(), SIGNAL( requestTimeSynchronization(int, unsigned long long, void *) ),
						 this, SLOT( performTimeSynchronization(int, unsigned long long, void *) ));
    connect(this, SIGNAL( requestTimeSynchronization(int, unsigned long long, void *) ),
						 &getSyncWidgets(), SLOT( performTimeSynchronization(int, unsigned long long, void *) ));
    connect(&getSyncWidgets(), SIGNAL( requestFrameSynchronization(int, unsigned long long, void *) ),
						 this, SLOT( performFrameSynchronization(int, unsigned long long, void *) ));
    connect(this, SIGNAL( requestFrameSynchronization(int, unsigned long long, void *) ),
						 &getSyncWidgets(), SLOT( performFrameSynchronization(int, unsigned long long, void *) ));
}


bool FrameView::handleAction (Action * a, E_ActionHandleType sync)
{
	return false;
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
	int const index = qRound(pa.x());

	if (index >= 0 && index < m_bars->m_values.size())
	{
		qDebug("clicked at frame=%i label=%s", index, m_bars->m_strvalues[index].toStdString().c_str());
		//unsigned long long n =  m_bars->m_begins[index];
		//emit requestTimeSynchronization(m_config.m_sync_group, n, this);
		emit requestFrameSynchronization(m_config.m_sync_group, index, this);
	}
}


void FrameView::performTimeSynchronization (int sync_group, unsigned long long time, void * source)
{
	qDebug("%s syncgrp=%i time=%i", __FUNCTION__, sync_group, time);
	// center on index
}

void FrameView::performFrameSynchronization (int sync_group, unsigned long long frame, void * source)
{
	qDebug("%s syncgrp=%i frame=%i", __FUNCTION__, sync_group, frame);
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
	//qDebug("frame: dt=%f", to - from);


	QwtInterval interval(0.0f, 500.0f);
	QwtLinearColorMap colormap(Qt::darkCyan, Qt::red);
	colormap.addColorStop(0.1, Qt::cyan);
	colormap.addColorStop(0.6, Qt::green);
	colormap.addColorStop(0.8, Qt::yellow);
	colormap.addColorStop(0.95, Qt::red);

	QColor const c = colormap.color(interval, to - from);

	m_bars->m_values.push_back(to - from);
	m_bars->m_begins.push_back(from);
	m_bars->m_ends.push_back(to);
	m_bars->m_colors.push_back(c);
	m_bars->m_strvalues.push_back(QString("%1").arg(m_bars->m_values.size()));

	m_bars->setSamples(m_bars->m_values);

	m_bars->itemChanged();
	m_bars->legendChanged();
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



