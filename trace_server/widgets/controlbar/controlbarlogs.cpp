#include "controlbarlogs.h"
#include "ui_controlbarlogs.h"

ControlBarLogs::ControlBarLogs(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlBarLogs)
{
    ui->setupUi(this);
}

ControlBarLogs::~ControlBarLogs()
{
    delete ui;
}
