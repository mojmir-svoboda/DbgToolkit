#pragma once
#include <QLabel>

struct VerticalLabel : QLabel {

	VerticalLabel (QWidget * parent) : QLabel(parent)
	{
		QFont f( "Verdana", 12, QFont::Bold);
		setFont(f);
		setAlignment(Qt::AlignCenter);
	}

	void paintEvent (QPaintEvent *)
	{
		QPainter painter(this);
		painter.setPen(Qt::black);
		QRectF const r = rect();
		painter.translate(r.width() / 2.0f + 12 / 2, r.height());
		painter.rotate(-90);
		painter.drawText(QPoint(0, 0), text());
	}
};

