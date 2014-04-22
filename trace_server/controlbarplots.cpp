#include "controlbarplots.h"
#include "ui_controlbarplots.h"

ControlBarPlots::ControlBarPlots(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlBarPlots)
{
    ui->setupUi(this);
}

ControlBarPlots::~ControlBarPlots()
{
    delete ui;
}
