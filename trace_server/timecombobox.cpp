#include "timecombobox.h"
#include "ui_timecombobox.h"

TimeComboBox::TimeComboBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeComboBox)
{
    ui->setupUi(this);
}

TimeComboBox::~TimeComboBox()
{
    delete ui;
}
