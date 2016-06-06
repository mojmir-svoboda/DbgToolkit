#include "combolist.h"
#include "ui_combolist.h"

ComboList::ComboList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ComboList)
{
    ui->setupUi(this);
}

ComboList::~ComboList()
{
    delete ui;
}
