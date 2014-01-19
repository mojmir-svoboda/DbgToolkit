#ifndef TIMECOMBOBOX_H
#define TIMECOMBOBOX_H

#include <QWidget>

namespace Ui {
class TimeComboBox;
}

class TimeComboBox : public QWidget
{
    Q_OBJECT
    
public:
    explicit TimeComboBox(QWidget *parent = 0);
    ~TimeComboBox();
    
private:
    Ui::TimeComboBox *ui;
};

#endif // TIMECOMBOBOX_H
