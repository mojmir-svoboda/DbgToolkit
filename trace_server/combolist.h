#ifndef COMBOLIST_H
#define COMBOLIST_H

#include <QDockWidget>

namespace Ui {
class ComboList;
}

class ComboList : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit ComboList(QWidget *parent = 0);
    ~ComboList();

    Ui::ComboList *ui;
};

#endif // COMBOLIST_H
