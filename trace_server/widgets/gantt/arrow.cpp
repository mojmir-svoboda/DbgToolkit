#include <QtGui>

#include "arrow.h"
#include <math.h>

const qreal Pi = 3.14;

Arrow::Arrow (QGraphicsItem * startItem, QGraphicsItem * endItem, QPointF const & pt0, QPointF const & pt1, QGraphicsItem * parent, QGraphicsScene * scene)
	: QGraphicsLineItem(parent)
	, m_startItem(startItem)
	, m_endItem(endItem)
	, m_pt0(pt0)
	, m_pt1(pt1)
{
	setFlag(QGraphicsItem::ItemIsSelectable, false);
	m_color = Qt::black;
	//setPen(QPen(m_color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	//setPen(QPen(m_color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

QRectF Arrow::boundingRect () const
{
	qreal const extra = (pen().width() + 20) / 2.0;
	return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(),
		line().p2().y() - line().p1().y()))
		.normalized()
		.adjusted(-extra, -extra, extra, extra);
}

QPainterPath Arrow::shape () const
{
	QPainterPath path = QGraphicsLineItem::shape();
	path.addPolygon(m_arrowHead);
	return path;
}

void Arrow::updatePosition ()
{
	QLineF const line(mapFromItem(m_startItem, 0, 0), mapFromItem(m_endItem, 0, 0));
	setLine(line);
}

void Arrow::paint (QPainter * painter, QStyleOptionGraphicsItem const *, QWidget *)
{
	if (m_startItem->collidesWithItem(m_endItem))
		return;

	qreal const arrowSize = 4;
	QPen p = pen();
	p.setColor(m_color);
	p.setWidth(0);
	painter->setPen(p);
	painter->setBrush(m_color);

	QPointF const & p1 = m_pt0;
	QPointF const & p2 = m_pt1;
	setLine(QLineF(p2, p1));

	double angle = ::acos(line().dx() / line().length());
	if (line().dy() >= 0)
		angle = (Pi * 2) - angle;

	QPointF const arrowP1 = line().p1() + QPointF(sin(angle + Pi / 3) * arrowSize, cos(angle + Pi / 3) * arrowSize);
	QPointF const arrowP2 = line().p1() + QPointF(sin(angle + Pi - Pi / 3) * arrowSize, cos(angle + Pi - Pi / 3) * arrowSize);
	m_arrowHead.clear();
	m_arrowHead << line().p1() << arrowP1 << arrowP2;
	painter->drawLine(line());
	painter->drawPolygon(m_arrowHead);

	if (isSelected())
	{
		painter->setPen(QPen(m_color, 1, Qt::DashLine));
		QLineF l = line();
		l.translate(0, 4.0);
		painter->drawLine(l);
		l.translate(0,-8.0);
		painter->drawLine(l);
	}
}
