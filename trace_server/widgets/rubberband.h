#pragma once
#include <QRubberBand>
#include <QPen>
#include <QStylePainter>

struct RubberBand : QRubberBand
{
	RubberBand (QRubberBand::Shape s, QWidget * p = nullptr) : QRubberBand (s, p) { }

	void paintEvent (QPaintEvent *)
	{
		QStylePainter painter(this);
		QStyleOptionRubberBand option;
		initStyleOption(&option);
		QPen pen;
		pen.setStyle(Qt::DashLine);
		pen.setWidth(2);
		if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))
			pen.setColor(QColor(Qt::green));
		if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier))
			pen.setColor(QColor(Qt::red));
		painter.setPen(pen);
		painter.drawRect(option.rect);
		//painter.drawControl(QStyle::CE_RubberBand, option);


	/*	option.initFrom(this);

		painter.begin(this);

		QPen pen;
		pen.setStyle(Qt::DashLine);
		pen.setWidth(2);
		pen.setColor(QColor(Qt::red));
		painter.setPen(pen);

		painter.drawControl(QStyle::CE_FocusFrame, option);*/
	}
};

