#include "ganttitem.h"
#include "ganttview.h"
#include <QtGui>
#include <QStyleOption>

namespace gantt {

BarTextItem::BarTextItem (Data & d, QColor const & color, int x, int y, int w, int h, int ctx, int ctx_offs)
	: m_data(d) 
	, m_x(x), m_y(y), m_w(w), m_h(h), m_color(color)
	, m_ctx(ctx), m_ctx_offs(ctx_offs)
{
	setZValue((x + y) % 2);
	setAcceptHoverEvents(true);
}

QRectF BarTextItem::boundingRect () const
{
	return QRectF(0, 0, m_w, m_h);
}

QPainterPath BarTextItem::shape () const
{
	QPainterPath path;
	path.addRect(0, 0, m_w, m_h);
	return path;
}

void BarTextItem::paint (QPainter * painter, QStyleOptionGraphicsItem const * option, QWidget * widget)
{
	Q_UNUSED(widget);

	qreal const lod = option->levelOfDetailFromTransform(painter->worldTransform());

	if (lod > 0.7f)
	{
		if (m_w > 10)
		{
			setFlag(QGraphicsItem::ItemIgnoresTransformations);
			QFont font("Times", 10);
			//QTransform identity;
			font.setStyleStrategy(QFont::ForceOutline);
			painter->save();
			painter->drawText(4, 10, QString("%1 [ %2 ms]").arg(m_data.m_msg).arg(m_data.m_delta_t / 1000.0f));
			painter->restore();
		}
	}
	else if (0.4f < lod && lod < 0.7f)
	{
		if (m_w > 20)
		{
			QFont font("Times", 8);
			font.setStyleStrategy(QFont::ForceOutline);
			painter->setFont(font);
			painter->save();
			painter->drawStaticText(4, m_h/2, QString("%1").arg(m_data.m_msg).arg(m_data.m_delta_t / 1000.0f));
			painter->restore();
		}
	}

}






BarItem::BarItem (Data & d, QColor const & color, int x, int y, int w, int h, int ctx, int ctx_offs)
	: m_data(d) 
	, m_x(x), m_y(y), m_w(w), m_h(h), m_color(color)
	, m_ctx(ctx), m_ctx_offs(ctx_offs)
{
	setZValue((x + y) % 2);
	//setFlags(ItemIsSelectable | ItemIsMovable);
	setFlags(ItemIsSelectable);
	setAcceptHoverEvents(true);
}

QRectF BarItem::boundingRect () const
{
	return QRectF(0, 0, m_w, m_h);
}

QPainterPath BarItem::shape () const
{
	QPainterPath path;
	path.addRect(0, 0, m_w, m_h);
	return path;
}

void BarItem::paint (QPainter * painter, QStyleOptionGraphicsItem const * option, QWidget * widget)
{
	Q_UNUSED(widget);

	QColor fillColor = (option->state & QStyle::State_Selected) ? m_color.dark(150) : m_color;
	if (option->state & QStyle::State_MouseOver)
	{
		fillColor = fillColor.light(125);
	}

	painter->fillRect(QRectF(0, 0, m_w, m_h), fillColor);

	qreal const lod = option->levelOfDetailFromTransform(painter->worldTransform());

	if (lod > 0.7f)
	{
		if (m_w > 10)
		{
			setFlag(QGraphicsItem::ItemIgnoresTransformations);
			QFont font("Times", 10);
			//QTransform identity;
			font.setStyleStrategy(QFont::ForceOutline);
			painter->save();
			painter->drawText(4, 10, QString("%1 [ %2 ms]").arg(m_data.m_msg).arg(m_data.m_delta_t / 1000.0f));
			painter->restore();
		}
	}
	else if (0.4f < lod && lod < 0.7f)
	{
		if (m_w > 20)
		{
			QFont font("Times", 8);
			font.setStyleStrategy(QFont::ForceOutline);
			painter->setFont(font);
			painter->save();
			painter->drawStaticText(4, m_h/2, QString("%1").arg(m_data.m_msg).arg(m_data.m_delta_t / 1000.0f));
			painter->restore();
		}
	}


/*	if (lod > 0.4f)
	{
		QFont font("Times", 8);
		font.setStyleStrategy(QFont::ForceOutline);
		painter->setFont(font);
		painter->save();
		painter->drawStaticText(4, g_heightValue/2, QString("%1").arg(m_block.m_msg.c_str()));
		painter->drawStaticText(4, 7 * g_heightValue/8, QString("%2 ms").arg(m_block.m_delta_t));
		painter->restore();
	}*/

/*
	if (lod >= 0.7f)
	{
		QFont font("Times", 8);
		font.setStyleStrategy(QFont::ForceOutline);
		painter->setFont(font);
		painter->save();
		painter->drawStaticText(4, g_heightValue/2, QString("%1").arg(m_block.m_msg.c_str()));
		painter->drawStaticText(4, 7 * g_heightValue/8, QString("%2 ms").arg(m_block.m_delta_t));
		painter->restore();
	}
	else if (0.4f < lod && lod < 0.7f)
	{
		if (m_w > 10)
		{
			QFont font("Times", 8);
			font.setStyleStrategy(QFont::ForceOutline);
			painter->setFont(font);
			painter->save();
			painter->drawStaticText(4, g_heightValue/2, QString("%1 %2").arg(m_block.m_msg.c_str()).arg(m_block.m_delta_t));
			painter->restore();
		}
	}
	else if (0.1f < lod && lod <= 0.4f)
	{
		if (m_w > 10)
		{
			QFont font("Times", 8);
			font.setStyleStrategy(QFont::ForceOutline);
			painter->setFont(font);
			painter->save();
			painter->drawStaticText(4, g_heightValue/2, QString("%1 %2").arg(m_block.m_msg.c_str()).arg(m_block.m_delta_t));
			painter->restore();
		}
	}*/

}

void BarItem::mousePressEvent (QGraphicsSceneMouseEvent * event)
{
}

void BarItem::mouseMoveEvent (QGraphicsSceneMouseEvent * event)
{
	QGraphicsItem::mouseMoveEvent(event);
}

void BarItem::mouseReleaseEvent (QGraphicsSceneMouseEvent * event)
{
}

/*void BarItem::hoverEnterEvent (QGraphicsSceneHoverEvent *event)
{
	qDebug("+");
}
void BarItem::hoverLeaveEvent (QGraphicsSceneHoverEvent *event)
{
	qDebug("-");
}*/

}

