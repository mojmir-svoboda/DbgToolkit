#ifndef PROFILERMAINWINDOW_H
#define PROFILERMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class ProfilerMainWindow;
}

class ProfilerMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit ProfilerMainWindow(QWidget *parent = 0);
    ~ProfilerMainWindow();
    
private:
    Ui::ProfilerMainWindow *ui;
};

#endif // PROFILERMAINWINDOW_H
