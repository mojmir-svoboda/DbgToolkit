#include "ganttitem.h"
#include "ganttview.h"
#include <QtGui>
#include <QStyleOption>

namespace gantt {

BarTextItem::BarTextItem (QGraphicsItem * parent, GanttViewConfig const & gvcfg, Data & d, QColor const & color, int x, int y, int w, int h, int ctx, int ctx_offs)
	: m_gvcfg(gvcfg)
    , m_data(d) 
	, m_x(x), m_y(y), m_w(w), m_h(h), m_color(color)
	, m_ctx(ctx), m_ctx_offs(ctx_offs)
{
	setZValue(1);
	setParentItem(parent);
	//setFlags(ItemIsSelectable);
	//setAcceptHoverEvents(true);
	setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
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


	BarItem const * b = static_cast<BarItem const *>(parentItem());
	qreal const lod = b ? b->lod() : 1.0f;

	if (lod < 0.2f)
		return;

	QRectF const rect = boundingRect();
	QPointF const pt = rect.translated(+1, +1).topLeft();
	
	if (lod > 0.7f)
	{
		painter->save();
		QFont font(m_gvcfg.m_font, m_gvcfg.m_fontsize);
		font.setStyleStrategy(QFont::ForceOutline);
		QString text = QString("%1 %2\n[ %3 ms]").arg(m_data.m_tag).arg(m_data.m_msg).arg(m_data.m_delta_t);
		painter->setFont(font);
		painter->setPen(Qt::black);
		painter->drawStaticText(pt, text);
		painter->restore();
	}
	else if (0.4f <= lod && lod < 0.7f)
	{
		painter->save();
		QFont font(m_gvcfg.m_font, m_gvcfg.m_fontsize - 1);
		font.setStyleStrategy(QFont::ForceOutline);
		painter->setFont(font);
		painter->setPen(Qt::black);
		painter->drawStaticText(pt, QString("%1 %2").arg(m_data.m_tag).arg(m_data.m_msg));
		painter->restore();
	}
	else if (0.2f <= lod && lod < 0.4f)
	{
		painter->save();
		QFont font(m_gvcfg.m_font, m_gvcfg.m_fontsize - 2);
		font.setStyleStrategy(QFont::ForceOutline);
		QString text = QString("%1").arg(m_data.m_tag);
		painter->setFont(font);
		painter->setPen(Qt::black);
		painter->drawText(pt, text);
		painter->restore();
	}
	else
	{
	}
}


BarItem::BarItem (GanttViewConfig const & cfg, Data & d, QColor const & color, int x, int y, int w, int h, int ctx, int ctx_offs)
	: m_gvcfg(cfg)
	, m_data(d) 
	, m_x(x), m_y(y), m_w(w), m_h(h), m_color(color)
	, m_ctx(ctx), m_ctx_offs(ctx_offs)
	, m_lod(1.0f)
{
	setZValue(0);
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
	QColor fillColor = (option->state & QStyle::State_Selected) ? m_color.dark(150) : m_color;
	if (option->state & QStyle::State_MouseOver)
	{
		fillColor = fillColor.light(125);
	}

	QRectF rect = boundingRect();
	painter->fillRect(QRectF(0, 0, m_w, m_h), fillColor);

	m_lod = option->levelOfDetailFromTransform(painter->worldTransform());
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

