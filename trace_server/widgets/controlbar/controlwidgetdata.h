#ifndef CONTROLWIDGETDATA_H
#define CONTROLWIDGETDATA_H

#include <QWidget>

namespace Ui {
class controlwidgetdata;
}

class controlwidgetdata : public QWidget
{
    Q_OBJECT

public:
    explicit controlwidgetdata(QWidget *parent = 0);
    ~controlwidgetdata();

private:
    Ui::controlwidgetdata *ui;
};

#endif // CONTROLWIDGETDATA_H
