#pragma once
#include <QPixmap>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

struct WarnImage : QPixmap
{
	QPixmap m_image;
	QPixmap m_image2;
	WarnImage ()
		: m_image(":images/find_stop.png")
	{
		init();
	}


	void init ()
	{
		QPixmap temp(m_image.size());
		temp.fill(Qt::transparent);
		 
		QPainter p(&temp);
		p.setCompositionMode(QPainter::CompositionMode_Source);
		p.drawPixmap(0, 0, m_image);
		p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
		int opacity = 128;
		p.fillRect(temp.rect(), QColor(0, 0, 0, opacity));
		p.end();
		 
		m_image2 = temp;
	}

	void warningFindNoMoreMatches ()
	{
		QGraphicsOpacityEffect * effect = new QGraphicsOpacityEffect(&m_image2);
		//QGraphicsOpacityEffect * effect = new QGraphicsOpacityEffect(m_image2);
		//m_image2.setGraphicsEffect(effect);
		QPropertyAnimation * anim = new QPropertyAnimation(effect, "opacity");
		anim->setStartValue(0.01);
		anim->setEndValue(1.0);
		anim->setDuration(5000);
		anim->start();
	}
};

