#pragma once
#include <QGraphicsLineItem>
#include "ganttitem.h"

class QGraphicsPolygonItem;
class QGraphicsLineItem;
class QGraphicsScene;
class QRectF;
class QGraphicsSceneMouseEvent;
class QPainterPath;

class Arrow : public QGraphicsLineItem
{
public:
	enum { Type = UserType + 4 };

	Arrow (QGraphicsItem * startItem, QGraphicsItem * endItem, QPointF const & pt0, QPointF const & pt1, QGraphicsItem * parent = 0, QGraphicsScene * scene = 0);

	int type () const { return Type; }
	QRectF boundingRect () const;
	QPainterPath shape () const;
	void setColor (QColor const & color) { m_color = color; }
	QGraphicsItem * startItem () const { return m_startItem; }
	QGraphicsItem * endItem () const { return m_endItem; }

	void updatePosition ();

protected:
	void paint (QPainter * painter, QStyleOptionGraphicsItem const * option, QWidget * widget = 0);

private:
	QGraphicsItem * m_startItem;
	QGraphicsItem * m_endItem;
	QPointF m_pt0;
	QPointF m_pt1;
	QColor m_color;
	QPolygonF m_arrowHead;
};

