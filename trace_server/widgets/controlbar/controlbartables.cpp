#include "controlbartables.h"
#include "ui_controlbartables.h"

ControlBarTables::ControlBarTables(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlBarTables)
{
    ui->setupUi(this);
}

ControlBarTables::~ControlBarTables()
{
    delete ui;
}
