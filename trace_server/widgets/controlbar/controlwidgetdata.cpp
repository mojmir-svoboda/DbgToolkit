#include "controlwidgetdata.h"
#include "ui_controlwidgetdata.h"

controlwidgetdata::controlwidgetdata(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::controlwidgetdata)
{
    ui->setupUi(this);
}

controlwidgetdata::~controlwidgetdata()
{
    delete ui;
}
