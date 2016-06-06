#include "controlbarcommon.h"
#include "ui_controlbarcommon.h"

ControlBarCommon::ControlBarCommon(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlBarCommon)
{
    ui->setupUi(this);
}

ControlBarCommon::~ControlBarCommon()
{
    delete ui;
}
