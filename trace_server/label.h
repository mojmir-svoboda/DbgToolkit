#pragma once
#include <QLabel>

struct VerticalLabel : QLabel {

	VerticalLabel (QWidget * parent) : QLabel(parent) { }

	void paintEvent (QPaintEvent *)
	{
		QPainter painter(this);
		painter.setPen(Qt::black);
		//... Need an appropriate call to painter.translate() for this to work properly
		painter.rotate(90);
		painter.drawText(QPoint(0,0), _text);
	}
};
