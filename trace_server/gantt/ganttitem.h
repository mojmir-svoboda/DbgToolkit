#pragma once
#include <QColor>
#include <QGraphicsItem>
#include "ganttdata.h"
#include "ganttconfig.h"

namespace gantt {

	class BarTextItem : public QGraphicsItem // @TODO: derive from QRectGraphicsItem? Or QTextGrItem?
	{
	public:
		BarTextItem (QGraphicsItem * parent, GanttViewConfig const & gvcfg, Data & d, QColor const & color, int x, int y, int w, int h, int ctx, int ctx_offs);

		QRectF boundingRect () const;
		QPainterPath shape () const;
		void paint (QPainter * painter, QStyleOptionGraphicsItem const * item, QWidget * widget);

	protected:
		GanttViewConfig const & m_gvcfg;
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
		BarItem (GanttViewConfig const & gvcfg, Data & d, QColor const & color, int x, int y, int w, int h, int ctx, int ctx_offs);

		QRectF boundingRect () const;
		QPainterPath shape () const;
		void paint (QPainter * painter, QStyleOptionGraphicsItem const * item, QWidget * widget);

		float lod () const { return m_lod; }

	protected:
		void mousePressEvent (QGraphicsSceneMouseEvent *event);
		void mouseMoveEvent (QGraphicsSceneMouseEvent *event);
		void mouseReleaseEvent (QGraphicsSceneMouseEvent *event);

	private:
		GanttViewConfig const & m_gvcfg;
		Data & m_data;
		int m_ctx;
		int m_ctx_offs;
		int m_x, m_y, m_w, m_h;
		QColor m_color;
		float m_lod;
		//QList<QPointF> m_stuff;
	};

	class FrameItem : public QGraphicsItem
	{
	public:
		FrameItem (GanttViewConfig const & gvcfg, QColor const & color, int x, int y, int w, int h, int ctx);

		QRectF boundingRect () const;
		QPainterPath shape () const;
		void paint (QPainter * painter, QStyleOptionGraphicsItem const * item, QWidget * widget);

		float lod () const { return m_lod; }

	protected:
		void mousePressEvent (QGraphicsSceneMouseEvent *event);
		void mouseMoveEvent (QGraphicsSceneMouseEvent *event);
		void mouseReleaseEvent (QGraphicsSceneMouseEvent *event);

	private:
		GanttViewConfig const & m_gvcfg;
		int m_ctx;
		int m_x, m_y, m_w, m_h;
		QColor m_color;
		float m_lod;
		//QList<QPointF> m_stuff;
	};

} 

