#ifndef CONTROLBARDOCKMANAGER_H
#define CONTROLBARDOCKMANAGER_H

#include <QWidget>

namespace Ui {
class ControlBarDockManager;
}

class ControlBarDockManager : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBarDockManager(QWidget *parent = 0);
    ~ControlBarDockManager();

    Ui::ControlBarDockManager *ui;
};

#endif // CONTROLBARDOCKMANAGER_H
