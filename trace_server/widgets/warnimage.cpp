#include "warnimage.h"
#include <QPainter>
#include <QTimer>
#include <QVBoxLayout>

WarnImage::WarnImage (QWidget * parent)
	: QWidget(parent)
	, m_image(0)
	, m_label(0)
	, m_timer(0)
	, m_anim(0)
	, m_eff(0)
{
	m_image = new QPixmap(":images/find_stop.png");
	m_label = new QLabel(this);
	m_eff = new QGraphicsOpacityEffect(this);
	setGraphicsEffect(m_eff);
	m_anim = new QPropertyAnimation(m_eff, "opacity");
	m_label->setPixmap(*m_image);
	QVBoxLayout * l = new QVBoxLayout(this);
	l->addWidget(m_label);
	setLayout(l);
	hide();
}

void WarnImage::warningFindNoMoreMatches ()
{
	show();
	connect(m_anim, SIGNAL(finished()), this, SLOT(onFinishedFwd()));
	m_anim->setStartValue(0.01);
	m_anim->setEndValue(1.0);
	m_anim->setDuration(42);
	m_anim->start();
}

void WarnImage::onFinishedFwd ()
{
	connect(m_anim, SIGNAL(finished()), this, SLOT(onFinishedBwd()));
	m_anim->setStartValue(1.00f);
	m_anim->setEndValue(0.0f);
	m_anim->setDuration(143);
	m_anim->start();
}

void WarnImage::onFinishedBwd ()
{
	hide();
	disconnect(m_anim, SIGNAL(finished()), this, SLOT(onFinishedFwd()));
	disconnect(m_anim, SIGNAL(finished()), this, SLOT(onFinishedBwd()));
}

