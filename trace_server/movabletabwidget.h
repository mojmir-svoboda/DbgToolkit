#pragma once
#include <QTabWidget>
#include <QTabBar>

struct MovableTabWidget : QTabWidget
{
Q_OBJECT
public:
	MovableTabWidget (QWidget * parent = 0);

signals:
	void tabMovedSignal (int from, int to);

public slots:
	void onTabMoved (int from, int to);
};

