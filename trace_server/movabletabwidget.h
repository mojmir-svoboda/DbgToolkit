#pragma once
#include <QTabWidget>
#include <QTabBar>

struct MovableTabWidget : QTabWidget
{
	MovableTabWidget (QWidget * parent = 0)
		: QTabWidget(parent)
	{
		setMovable(true);
		connect(tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(onTabMoved(int,int)));
	}
signals:
	void tabMovedSignal (int from, int to);

public slots:
	void onTabMoved (int from, int to)
	{
		emit tabMovedSignal(from, to);
	}
	Q_OBJECT
};

