#include "controlbarlog.h"
#include "ui_controlbarlog.h"

ControlBarLog::ControlBarLog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlBarLog)
{
    ui->setupUi(this);
}

ControlBarLog::~ControlBarLog()
{
    delete ui;
}
