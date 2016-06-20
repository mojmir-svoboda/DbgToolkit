#include "controlwidgetdata.h"
#include "ui_controlwidgetdata.h"
#include <connectionconfig.h>

ControlWidgetData::ControlWidgetData(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlWidgetData)
{
    ui->setupUi(this);
}

ControlWidgetData::~ControlWidgetData()
{
    delete ui;
}

void ControlWidgetData::applyConfig (ConnectionConfig & cfg)
{
	//m_config = config;
	ui->logSlider->setValue(cfg.m_logs_recv_level);
	ui->plotSlider->setValue(cfg.m_plots_recv_level);
	ui->tableSlider->setValue(cfg.m_tables_recv_level);
	ui->ganttSlider->setValue(cfg.m_gantts_recv_level);
}
