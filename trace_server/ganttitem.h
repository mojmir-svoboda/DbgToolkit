#pragma once
#include <QColor>
#include <QGraphicsItem>
#include "ganttdata.h"

namespace gantt {

	class BarTextItem : public QGraphicsItem // @TODO: derive from QRectGraphicsItem? Or QTextGrItem?
	{
	public:
		BarTextItem (Data & d, QColor const & color, int x, int y, int w, int h, int ctx, int ctx_offs);

		QRectF boundingRect () const;
		QPainterPath shape () const;
		void paint (QPainter * painter, QStyleOptionGraphicsItem const * item, QWidget * widget);

	protected:
		Data & m_data;
		int m_ctx;
		int m_ctx_offs;
		int m_x, m_y, m_w, m_h;
		QColor m_color;
	private:
	};


	class BarItem : public QGraphicsItem
	{
	public:
		BarItem (Data & d, QColor const & color, int x, int y, int w, int h, int ctx, int ctx_offs);

		QRectF boundingRect () const;
		QPainterPath shape () const;
		void paint (QPainter * painter, QStyleOptionGraphicsItem const * item, QWidget * widget);

	protected:
		void mousePressEvent (QGraphicsSceneMouseEvent *event);
		void mouseMoveEvent (QGraphicsSceneMouseEvent *event);
		void mouseReleaseEvent (QGraphicsSceneMouseEvent *event);

	private:
		Data & m_data;
		int m_ctx;
		int m_ctx_offs;
		int m_x, m_y, m_w, m_h;
		QColor m_color;
		//QList<QPointF> m_stuff;
	};

} 

