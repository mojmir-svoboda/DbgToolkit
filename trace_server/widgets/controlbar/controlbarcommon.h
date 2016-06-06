#pragma once
#include <QWidget>

namespace Ui { class ControlBarCommon; }

class ControlBarCommon : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBarCommon(QWidget *parent = 0);
    ~ControlBarCommon();

    Ui::ControlBarCommon *ui;
};

