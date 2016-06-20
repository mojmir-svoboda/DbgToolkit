#include "controlbardockmanager.h"
#include "ui_controlbardockmanager.h"

ControlBarDockManager::ControlBarDockManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlBarDockManager)
{
    ui->setupUi(this);
}

ControlBarDockManager::~ControlBarDockManager()
{
    delete ui;
}
