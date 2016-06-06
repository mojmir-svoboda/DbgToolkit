#include "controlbar_dockmanager.h"

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

