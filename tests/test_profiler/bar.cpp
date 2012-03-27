#include "bar.h"
#include "view.h"
#include <QtGui>

Bar::Bar (BlockInfo & bi, QColor const & color, int x, int y, int w, int h)
	: m_block(bi) 
	, m_x(x), m_y(y), m_w(w), m_h(h), m_color(color)
{
	setZValue((x + y) % 2);
	//setFlags(ItemIsSelectable | ItemIsMovable);
	setFlags(ItemIsSelectable);
	setAcceptsHoverEvents(true);
}

QRectF Bar::boundingRect () const
{
	return QRectF(0, 0, m_w, g_heightValue);
}

QPainterPath Bar::shape () const
{
	QPainterPath path;
	path.addRect(0, 0, m_w, g_heightValue);
	return path;
}

void Bar::paint (QPainter * painter, QStyleOptionGraphicsItem const * option, QWidget * widget)
{
	Q_UNUSED(widget);

	QColor fillColor = (option->state & QStyle::State_Selected) ? m_color.dark(150) : m_color;
	if (option->state & QStyle::State_MouseOver)
	{
		fillColor = fillColor.light(125);
	}

	painter->fillRect(QRectF(0, 0, m_w, g_heightValue), fillColor);

	qreal const lod = option->levelOfDetailFromTransform(painter->worldTransform());
	if (lod >= 0.7f)
	{
		QFont font("Times", 10);
		font.setStyleStrategy(QFont::ForceOutline);
		painter->setFont(font);
		painter->save();
		painter->drawText(4, g_heightValue/2, QString("%1").arg(m_block.m_msg.c_str()));
		painter->drawText(4, 7 * g_heightValue/8, QString("%2 ms").arg(m_block.m_delta_t));
		painter->restore();
	}
	else if (0.4f < lod && lod < 0.7f)
	{
		QFont font("Times", 16);
		font.setStyleStrategy(QFont::ForceOutline);
		painter->setFont(font);
		painter->save();
		painter->drawText(4, g_heightValue/2, QString("%1 %2").arg(m_block.m_msg.c_str()).arg(m_block.m_delta_t));
		painter->restore();
	}
}

void Bar::mousePressEvent (QGraphicsSceneMouseEvent * event)
{
}

void Bar::mouseMoveEvent (QGraphicsSceneMouseEvent * event)
{
	QGraphicsItem::mouseMoveEvent(event);
}

void Bar::mouseReleaseEvent (QGraphicsSceneMouseEvent * event)
{
}

/*void Bar::hoverEnterEvent (QGraphicsSceneHoverEvent *event)
{
	qDebug("+");
}
void Bar::hoverLeaveEvent (QGraphicsSceneHoverEvent *event)
{
	qDebug("-");
}*/
