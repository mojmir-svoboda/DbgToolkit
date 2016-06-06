#include "controlbargantts.h"
#include "ui_controlbargantts.h"

ControlBarGantts::ControlBarGantts(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlBarGantts)
{
    ui->setupUi(this);
}

ControlBarGantts::~ControlBarGantts()
{
    delete ui;
}
