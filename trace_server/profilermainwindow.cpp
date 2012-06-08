#include "profilermainwindow.h"
#include "ui_profilermainwindow.h"

ProfilerMainWindow::ProfilerMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProfilerMainWindow)
{
    ui->setupUi(this);
}

ProfilerMainWindow::~ProfilerMainWindow()
{
    delete ui;
}
