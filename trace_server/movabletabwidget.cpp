#include "movabletabwidget.h"

MovableTabWidget::MovableTabWidget (QWidget * parent)
	: QTabWidget(parent)
{
	setMovable(true);
	connect(tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(onTabMoved(int,int)));
}


void MovableTabWidget::onTabMoved (int from, int to)
{
	emit tabMovedSignal(from, to);
}
