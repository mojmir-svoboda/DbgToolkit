#pragma once
#include <QPixmap>
#include <QWidget>
#include <QLabel>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

struct WarnImage : QWidget
{
	QPixmap * 	m_image;
	QLabel * 	m_label;
	QTimer * 	m_timer;
	QPropertyAnimation * m_anim;
	QGraphicsOpacityEffect * m_eff;

	WarnImage (QWidget * parent = 0);
	void warningFindNoMoreMatches ();

Q_OBJECT
public slots:
	void onFinishedFwd ();
	void onFinishedBwd ();
};

