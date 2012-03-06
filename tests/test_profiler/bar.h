#pragma once
#include <QtGui/QColor>
#include <QtGui/QGraphicsItem>
#include "blockinfo.h"

class Bar : public QGraphicsItem
{
public:
    Bar (BlockInfo & bi, QColor const & color, int x, int y, int w, int h);

    QRectF boundingRect () const;
    QPainterPath shape () const;
    void paint (QPainter * painter, QStyleOptionGraphicsItem const * item, QWidget * widget);

protected:
    void mousePressEvent (QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent (QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent (QGraphicsSceneMouseEvent *event);

private:
	BlockInfo & m_block;
    int m_x, m_y, m_w, m_h;
    QColor m_color;
    QList<QPointF> m_stuff;
};

