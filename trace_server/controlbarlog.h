#ifndef CONTROLBARLOG_H
#define CONTROLBARLOG_H

#include <QWidget>

namespace Ui {
class ControlBarLog;
}

class ControlBarLog : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBarLog(QWidget *parent = 0);
    ~ControlBarLog();

private:
    Ui::ControlBarLog *ui;
};

#endif // CONTROLBARLOG_H
