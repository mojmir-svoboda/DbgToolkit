#pragma once
#include <QLabel>

struct VerticalLabel : QLabel {

	VerticalLabel (QWidget * parent) 
		: QLabel(parent)
		, m_fontSize(12)
		, m_font("Verdana", m_fontSize)
		, m_metrics(m_font)
		, m_color(Qt::blue)
	{
		setFont(m_font);
		//setAlignment(Qt::AlignCenter);
	}

	void paintEvent (QPaintEvent *)
	{
		QPainter painter(this);
		painter.setPen(m_color);
		QRectF const r = rect();
		int const tw = m_metrics.width(text());
		QRect const tr = m_metrics.boundingRect(text());
		int const th = tr.height();
		painter.translate(r.width() / 2.0f + r.width() / 4, r.height() - r.height() / 2 + th);
		painter.rotate(-90);
		painter.drawText(QPoint(0, 0), text());
	}

	int m_fontSize;
	QFont m_font;
	QFontMetrics m_metrics;
	QColor m_color;
};

