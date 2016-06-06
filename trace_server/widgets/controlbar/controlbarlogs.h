#ifndef CONTROLBARLOGS_H
#define CONTROLBARLOGS_H

#include <QWidget>

namespace Ui {
class ControlBarLogs;
}

class ControlBarLogs : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBarLogs(QWidget *parent = 0);
    ~ControlBarLogs();

private:
    Ui::ControlBarLogs *ui;
};

#endif // CONTROLBARLOGS_H
