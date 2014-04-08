#pragma once
#include <qtsln/flowlayout.h>

struct ButtonCache : FlowLayout
{
    ButtonCache (QWidget * parent = 0) : FlowLayout(parent, 0, 0, 0) { }
Q_OBJECT
};


