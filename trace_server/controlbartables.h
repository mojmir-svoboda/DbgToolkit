#ifndef CONTROLBARTABLES_H
#define CONTROLBARTABLES_H

#include <QWidget>

namespace Ui {
class ControlBarTables;
}

class ControlBarTables : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBarTables(QWidget *parent = 0);
    ~ControlBarTables();

private:
    Ui::ControlBarTables *ui;
};

#endif // CONTROLBARTABLES_H
