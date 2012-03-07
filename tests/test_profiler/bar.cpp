#include "bar.h"
#include <QtGui>

Bar::Bar (BlockInfo & bi, QColor const & color, int x, int y, int w, int h)
	: m_block(bi) 
	, m_x(x), m_y(y), m_w(w), m_h(h), m_color(color)
{
	setZValue((x + y) % 2);
	setFlags(ItemIsSelectable | ItemIsMovable);
	setAcceptsHoverEvents(true);
}

QRectF Bar::boundingRect () const
{
	return QRectF(0, 0, m_w, m_h);
}

QPainterPath Bar::shape () const
{
	QPainterPath path;
	path.addRect(0, 0, m_w, m_h);
	return path;
}

void Bar::paint (QPainter * painter, QStyleOptionGraphicsItem const * option, QWidget * widget)
{
	Q_UNUSED(widget);

	QColor fillColor = (option->state & QStyle::State_Selected) ? m_color.dark(150) : m_color;
	if (option->state & QStyle::State_MouseOver)
		fillColor = fillColor.light(125);

	painter->fillRect(QRectF(0, 0, m_w, m_h), fillColor);

	qreal const lod = option->levelOfDetailFromTransform(painter->worldTransform());

	if (lod >= 0.4f)
	{
		QFont font("Times", 9);
		font.setStyleStrategy(QFont::ForceOutline);
		painter->setFont(font);
		painter->save();
		painter->drawText(4, m_h/2, QString("%1").arg(m_block.m_msg.c_str()));
		painter->drawText(4, 7 * m_h/8, QString("%2 ms").arg(m_block.m_delta_t));
		painter->restore();
	}
}

void Bar::mousePressEvent (QGraphicsSceneMouseEvent * event)
{
}

void Bar::mouseMoveEvent (QGraphicsSceneMouseEvent * event)
{
}

void Bar::mouseReleaseEvent (QGraphicsSceneMouseEvent * event)
{
}
