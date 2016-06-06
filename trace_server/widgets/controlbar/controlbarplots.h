#ifndef CONTROLBARPLOTS_H
#define CONTROLBARPLOTS_H

#include <QWidget>

namespace Ui {
class ControlBarPlots;
}

class ControlBarPlots : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBarPlots(QWidget *parent = 0);
    ~ControlBarPlots();

private:
    Ui::ControlBarPlots *ui;
};

#endif // CONTROLBARPLOTS_H
