#ifndef CONTROLBARCOMMON_H
#define CONTROLBARCOMMON_H

#include <QWidget>

namespace Ui {
class ControlBarCommon;
}

class ControlBarCommon : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBarCommon(QWidget *parent = 0);
    ~ControlBarCommon();

private:
    Ui::ControlBarCommon *ui;
};

#endif // CONTROLBARCOMMON_H
