#ifndef CONTROLBARGANTTS_H
#define CONTROLBARGANTTS_H

#include <QWidget>

namespace Ui {
class ControlBarGantts;
}

class ControlBarGantts : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBarGantts(QWidget *parent = 0);
    ~ControlBarGantts();

private:
    Ui::ControlBarGantts *ui;
};

#endif // CONTROLBARGANTTS_H
