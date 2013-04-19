#include "ganttview.h"
#include <QtGui>
#include <QSpinBox>
#ifndef QT_NO_OPENGL
#	include <QtOpenGL>
#endif
#include <qmath.h>
#include "ganttitem.h"
#include "hsv.h"

namespace gantt {

int g_heightValue = 38;
int g_spaceValue = 15;

void GanttView::initColors ()
{
	// pick colors for unique clusters
	m_unique_colors.reserve(m_max_unique_colors);
	for (size_t hi = 0; hi < 360; hi += 360 / m_max_unique_colors)
	{
		HSV hsv;
		hsv.h = hi / 360.0f;
		hsv.s = 0.70f + tmp_randf() * 0.2f - 0.05f;
		hsv.v = 0.85f + tmp_randf() * 0.2f - 0.05f;
		QColor qcolor;
		qcolor.setHsvF(hsv.h, hsv.s, hsv.v);
		m_unique_colors.push_back(qcolor);
	}
}

GfxView & GanttView::createViewForContext (unsigned long long ctx, QGraphicsScene * s)
{
	contextviews_t::iterator it = m_contextviews.find(ctx);
	if (it == m_contextviews.end())		
	{
		GraphicsView * view = new GraphicsView();
		view->setRenderHint(QPainter::Antialiasing, false);
		view->setDragMode(QGraphicsView::RubberBandDrag);
		view->setOptimizationFlags(QGraphicsView::DontSavePainterState);
		view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
		m_layout->addWidget(view, 0, 0);

		QGraphicsScene * scene = (s == 0) ? new QGraphicsScene() : s;
		GfxView g;
		g.m_view = view;
		g.m_scene = scene;

		it = m_contextviews.insert(ctx, g);
		return *it;
	}
	return *it;
}

GfxView & GanttView::viewAt (unsigned long long ctx)
{
	contextviews_t::iterator it = m_contextviews.find(ctx);
	if (it == m_contextviews.end())
		return createViewForContext(ctx, NULL);
	return *it;
}

void GanttView::appendFrameBgn (DecodedData & dd)
{
	m_ganttData.m_frame_begin = dd.m_time;
	m_ganttData.m_completed_frame_data.push_back(new contextdata_t()); // @TODO: reserve

	for(size_t i = 0, ie = m_ganttData.m_contexts.size(); i < ie; ++i)
	{
		m_ganttData.m_completed_frame_data.back()->push_back(data_t()); // @TODO: reserve
	}

	++m_ganttData.m_frame;
}

void GanttView::appendFrameEnd (DecodedData & dd)
{
	float const scale = m_gvcfg.m_timescale;
	m_ganttData.m_frames.push_back(std::make_pair(m_ganttData.m_frame_begin / scale, dd.m_time / scale));

	size_t const from = m_last_flush_end_idx;
	size_t const to = m_ganttData.m_completed_frame_data.size();
	qDebug("flushing from %i to %i", from, to);

	for (size_t i = from; i < to; ++i)
	{
		qDebug("producing[%i], sz=%u", i, m_ganttData.m_completed_frame_data[i]->size());
		consumeData(m_ganttData.m_completed_frame_data[i]);
	}

	//dump
	/*for (size_t i = from; i < to; ++i)
		for (size_t j = 0, je = m_ganttData.m_completed_frame_infos[i]->size(); j < je; ++j)
			qDebug("producing item[%i]=0x%016x, contexts=%u bis_sz=%u", i, m_ganttData.m_completed_frame_infos[i], m_ganttData.m_completed_frame_infos[i]->size(),  m_ganttData.m_completed_frame_infos[i]->operator[](j).size());
*/
	m_last_flush_end_idx = to;
	//emit incomingProfilerData(&m_rvp);

}
void GanttView::appendBgn (DecodedData & dd)
{
	Data * prev = 0;
	if (m_ganttData.m_pending_data[dd.m_ctx_idx].size())
		prev = m_ganttData.m_pending_data[dd.m_ctx_idx].back();
	m_ganttData.m_data_ptrs.push_back(new Data());
	m_ganttData.m_pending_data[dd.m_ctx_idx].push_back(m_ganttData.m_data_ptrs.back());
	Data & d = *m_ganttData.m_pending_data[dd.m_ctx_idx].back();
	d.m_time_bgn = dd.m_time;
	d.m_ctx = dd.m_ctx;
	d.m_ctx_idx = dd.m_ctx_idx;
	d.m_msg = dd.m_text;
	d.m_layer = m_ganttData.m_pending_data[dd.m_ctx_idx].size() - 1;
	d.m_frame = m_ganttData.m_frame;
	d.m_parent = prev;

	if (d.m_msg.isEmpty())
		qDebug("wtf");
	qDebug("+++ f=%i t=%8llu  ctx=%llu  msg=%s", d.m_frame, d.m_time_bgn, d.m_ctx, d.m_msg.toStdString().c_str());
}

void GanttView::appendEnd (DecodedData & dd)
{
	unsigned const frame_idx = m_ganttData.m_frame;
	Data * d = m_ganttData.m_pending_data[dd.m_ctx_idx].back();
	m_ganttData.m_pending_data[dd.m_ctx_idx].pop_back();

	d->m_time_end = dd.m_time;
	d->m_msg.append(dd.m_text);
	d->complete();

	(*m_ganttData.m_completed_frame_data[d->m_frame])[dd.m_ctx_idx].push_back(d);

	qDebug("--- f=%i t=%8llu  ctx=%llu  msg=%s", d->m_frame, d->m_time_bgn, d->m_ctx, d->m_msg.toStdString().c_str());
}

void GanttView::appendGantt (DecodedData & dd)
{
	std::vector<unsigned long long>::iterator it = std::find(m_ganttData.m_contexts.begin(), m_ganttData.m_contexts.end(), dd.m_ctx);
	if (it == m_ganttData.m_contexts.end())
	{
		dd.m_ctx_idx = m_ganttData.m_contexts.size();
		m_ganttData.m_contexts.push_back(dd.m_ctx);
		m_ganttData.m_completed_frame_data[m_ganttData.m_frame]->push_back(data_t()); // @TODO: reserve
		m_ganttData.m_pending_data.push_back(data_t());
	}
	else
	{
		dd.m_ctx_idx = std::distance(m_ganttData.m_contexts.begin(), it);
	}


	typedef void (GanttView::*ptr) (DecodedData & dd);
	ptr ptrs[] = { &GanttView::appendBgn, &GanttView::appendEnd, &GanttView::appendFrameBgn, &GanttView::appendFrameEnd };

	if (dd.m_type < e_max_ganttcmd_enum_value)
	{
		(this->*ptrs[dd.m_type])(dd);
	}
}

void GanttView::consumeData (contextdata_t * c)
{
	contextdata_t & contexts = *c;
	qDebug("consumed node: contexts_sz=%u", contexts.size());
	for (size_t ci = 0, te = contexts.size(); ci < te; ++ci)
	{
		GfxView & v = viewAt(ci);
		data_t const & datas = contexts[ci];
		qDebug("processing data:, contexts_sz=%u datas_sz=%u", contexts.size(), datas.size());
		for (size_t di = 0, die = datas.size(); di < die; ++di)
		{
			Data & d = *datas[di];
			d.m_tag = d.m_msg;
			int const l = d.m_tag.lastIndexOf('[');
			int const r = d.m_tag.lastIndexOf(']');
			if (l != -1 && r != -1)
				d.m_tag.chop(l);
			qDebug("consumed data: contexts_sz=%u datas_sz=%u di=%u d_msg=%s d_tag=%s", contexts.size(), datas.size(), di, d.m_msg.toStdString().c_str(), d.m_tag.toStdString().c_str());

			colormap_t::iterator it = m_tagcolors.find(d.m_tag);
			if (it == m_tagcolors.end())
			{
				if (m_unique_colors.size() > 0)
				{
					d.m_color = m_unique_colors.back();
					m_unique_colors.pop_back();
				}
				m_tagcolors[d.m_tag] = d.m_color;
			}

			if (d.m_layer >= m_max_layers[ci])
				m_max_layers[ci] = d.m_layer;
		}

		int max_y = 0;
		int max_x = 0;

		int const h = g_heightValue;
		int const space = g_spaceValue;
		unsigned offs = 1;

		for (size_t di = 0, be = datas.size(); di < be; ++di)
		{
			Data & d = *datas[di];

			//registerTag(d);

			int w = d.m_dt;
			qreal x = d.m_time_bgn;
			qreal y = (offs) * (h + space)  + d.m_layer * (h + space);
			d.m_x = x / m_gvcfg.m_timescale;
			d.m_y = y;
			qDebug("f=%2u ci=%2u di=%2u  %s   (%3.2f, %3.2f) (x=%6.1f y=%6.1f w=%4i h=%4i dt=%3.3f)\n", d.m_frame, ci, di, d.m_tag.toStdString().c_str(), d.m_msg.toStdString().c_str(), d.m_x, d.m_y, x, y, w, h, d.m_dt);
			//fflush(stdout);

			if (y > max_y)
				max_y = y;

			if (x > max_x)
				max_x = x;

			QGraphicsItem * item = new BarItem(d, d.m_color, 0, 0, w, h, ci, offs);
			item->setPos(QPointF(d.m_x, y));
			v.m_scene->addItem(item);
			item->setToolTip(QString("frame=%1 thread=%2 %3 [%4 ms]").arg(d.m_frame).arg(ci).arg(d.m_msg).arg(d.m_dt / 1000.0f));

			QGraphicsItem * titem = new BarTextItem(d, d.m_color, 0, 0, w, h, ci, offs);
			titem->setPos(QPointF(d.m_x, y));
			v.m_scene->addItem(titem);
		}

		offs += m_max_layers[ci];
	}

	int const h = g_heightValue;
	int const space = g_spaceValue;
	for (size_t ci = 0, te = contexts.size(); ci < te; ++ci)
	{
		GfxView & v = viewAt(ci);
		data_t const & datas = contexts[ci];

		for (size_t di = 0, be = datas.size(); di < be; ++di)
		{
			Data const & d = *datas[di];
			if (d.m_parent)
			{
				if (d.m_parent->m_x < 100.0f || d.m_parent->m_y < 100.0f)
				{
					// incomplete parent!
				}
				else
				{
					QPen p1;
					p1.setColor(Qt::blue);
					p1.setWidth(0);
					QGraphicsLineItem * ln_bg = new QGraphicsLineItem(d.m_x, d.m_y, d.m_parent->m_x, d.m_parent->m_y + g_heightValue);
					ln_bg->setPen(p1);
					v.m_scene->addItem(ln_bg);
					QGraphicsLineItem * ln_nd = new QGraphicsLineItem(d.m_x + d.m_dt, d.m_y, d.m_parent->m_x + d.m_parent->m_dt, d.m_parent->m_y + g_heightValue);
					p1.setColor(Qt::cyan);
					ln_nd->setPen(p1);
					v.m_scene->addItem(ln_nd);
				}
			}

			QPen p1;
			p1.setColor(Qt::gray);
			QGraphicsLineItem * ln_end = new QGraphicsLineItem(d.m_x + d.m_dt, d.m_y, d.m_x + d.m_dt, d.m_y + g_heightValue);
			ln_end->setPen(p1);
			p1.setWidth(4);
			v.m_scene->addItem(ln_end);
		}
	} 

	for (size_t ci = 0, cie = contexts.size(); ci < cie; ++ci)
	{
		GfxView & v = viewAt(ci);
		//v.m_view->forceUpdate();
	}
}




GanttView::GanttView (Connection * conn, QWidget * parent, gantt::GanttViewConfig & config, QString const & fname)
	: QFrame(parent)
	, m_connection(conn)
	, m_gvcfg(config)
	, m_last_flush_end_idx(0)
{
	qDebug("%s", __FUNCTION__);

	setFrameStyle(Sunken | StyledPanel);

	m_layout = new QGridLayout;
	setLayout(m_layout);

	//connect(m_resetButton, SIGNAL(clicked()), this, SLOT(resetView()));
	//connect(m_zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
	//connect(m_graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(setResetButtonEnabled()));
	//connect(m_graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(setResetButtonEnabled()));
	//connect(m_antialiasButton, SIGNAL(toggled(bool)), this, SLOT(toggleAntialiasing()));
	//connect(m_openGlButton, SIGNAL(toggled(bool)), this, SLOT(toggleOpenGL()));
	//connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
	//connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));

	//connect(m_heightSlider, SIGNAL(valueChanged(int)), this, SLOT(changeHeight(int)));

	setupMatrix();
	m_ganttData.m_completed_frame_data.push_back(new contextdata_t()); // @TODO: reserve
}

void GanttView::resetView()
{
	//m_zoomSlider->setValue(250);
	setupMatrix();

	for (contextviews_t::iterator it = m_contextviews.begin(), ite = m_contextviews.end(); it != ite; ++it)
		(*it).m_view->ensureVisible(QRectF(0, 0, 0, 0));

	//m_resetButton->setEnabled(false);
}

void GanttView::changeHeight (int n)
{
/*	g_heightValue = n;
	QGraphicsScene * scene = view()->scene();
	view()->setScene(0);

	scene->clear();*/
	//m_mainWindow->populateScene();
	//view()->setScene(scene);
	//m_graphicsView->viewport()->update();
}

void GanttView::setResetButtonEnabled()
{
	//m_resetButton->setEnabled(true);
}

void GanttView::setupMatrix()
{
	qreal scale = qPow(qreal(2), (m_gvcfg.m_zoom - 250.0f) / qreal(50));

	QMatrix matrix;
	matrix.scale(scale, scale);

	for (contextviews_t::iterator it = m_contextviews.begin(), ite = m_contextviews.end(); it != ite; ++it)
		(*it).m_view->setMatrix(matrix);

	setResetButtonEnabled();
}

void GanttView::toggleOpenGL()
{
#ifndef QT_NO_OPENGL
	//m_graphicsView->setViewport(m_openGlButton->isChecked() ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
#endif
}

void GanttView::toggleAntialiasing()
{
	//m_graphicsView->setRenderHint(QPainter::Antialiasing, m_antialiasButton->isChecked());
}

void GanttView::zoomIn()
{
	m_gvcfg.m_zoom += 1.0f;
}

void GanttView::zoomOut()
{
	m_gvcfg.m_zoom -= 1.0f;
}

void GanttView::forceUpdate ()
{
	for (contextviews_t::iterator it = m_contextviews.begin(), ite = m_contextviews.end(); it != ite; ++it)
		(*it).m_view->viewport()->update();
}

GraphicsView::GraphicsView (QWidget * parent)
	: QGraphicsView(parent)
{ }

/**
  * Sets the current centerpoint.  Also updates the scene's center point.
  * Unlike centerOn, which has no way of getting the floating point center
  * back, SetCenter() stores the center point.	It also handles the special
  * sidebar case.  This function will claim the centerPoint to sceneRec ie.
  * the centerPoint must be within the sceneRec.
  */
void GraphicsView::SetCenter (QPointF const & centerPoint)
{
	// Get the rectangle of the visible area in scene coords
	QRectF visibleArea = mapToScene(rect()).boundingRect();
	// Get the scene area
	QRectF sceneBounds = sceneRect();

	double const boundX = visibleArea.width() / 2.0;
	double const boundY = visibleArea.height() / 2.0;
	double const boundWidth = sceneBounds.width() - 2.0 * boundX;
	double const boundHeight = sceneBounds.height() - 2.0 * boundY;

	//qDebug("setcenter: x=%f y=%f w=%f h=%f", boundX, boundY, boundWidth, boundHeight);
	// The max boundary that the centerPoint can be to
	QRectF bounds(boundX, boundY, boundWidth, boundHeight);
	if (bounds.contains(centerPoint))
	{
		// We are within the bounds
		CurrentCenterPoint = centerPoint;
	}
	else
	{
		// We need to clamp or use the center of the screen
		if (visibleArea.contains(sceneBounds))
		{
			// Use the center of scene ie. we can see the whole scene
			CurrentCenterPoint = centerPoint;
			//CurrentCenterPoint = sceneBounds.center();
		}
		else
		{
			CurrentCenterPoint = centerPoint;
 
			//We need to clamp the center. The centerPoint is too large
			/*if (centerPoint.x() > bounds.x() + bounds.width()) {
				CurrentCenterPoint.setX(bounds.x() + bounds.width());
			} else if (centerPoint.x() < bounds.x()) {
				CurrentCenterPoint.setX(bounds.x());
			}
 
			if (centerPoint.y() > bounds.y() + bounds.height()) {
				CurrentCenterPoint.setY(bounds.y() + bounds.height());
			} else if (centerPoint.y() < bounds.y()) {
				CurrentCenterPoint.setY(bounds.y());
			}*/
		}
	}
	// Update the scrollbars
	centerOn(CurrentCenterPoint);
}
 
void GraphicsView::mousePressEvent (QMouseEvent * event)
{
	// For panning the view
	LastPanPoint = event->pos();
	setCursor(Qt::ClosedHandCursor);
}
 
void GraphicsView::mouseReleaseEvent (QMouseEvent * event)
{
	setCursor(Qt::OpenHandCursor);
	LastPanPoint = QPoint();
}
 
void GraphicsView::mouseMoveEvent (QMouseEvent * event)
{
	if (!LastPanPoint.isNull())
	{
		//Get how much we panned
		QPointF delta = mapToScene(LastPanPoint) - mapToScene(event->pos());
		LastPanPoint = event->pos();

		QPointF cen = mapToScene(viewport()->rect()).boundingRect().center();
		SetCenter(cen + delta);
 
		//Update the center ie. do the pan
		//SetCenter(GetCenter() + delta);
		//qDebug("new center: %f %f", GetCenter().x(), GetCenter().y()); 
	}
	else
	{
		QGraphicsView::mouseMoveEvent(event);
	}
}
 
/**
  * Zoom the view in and out.
  */
void GraphicsView::wheelEvent (QWheelEvent* event)
{
	bool const shift = event->modifiers() & Qt::SHIFT;

	if (shift)
	{
		//m_frameSpinBox.setValue(m_frameSpinBox.value() + event->delta());
	}
	else
	{
		//Get the position of the mouse before scaling, in scene coords
		QPointF pointBeforeScale(mapToScene(event->pos()));

		//Get the original screen centerpoint
		QPointF screenCenter = GetCenter(); //CurrentCenterPoint; //(visRect.center());

		//Scale the view ie. do the zoom
		double scaleFactor = 1.15; //How fast we zoom
		if (event->delta() > 0) {
			//Zoom in
			scale(scaleFactor, scaleFactor);
		} else {
			//Zooming out
			scale(1.0 / scaleFactor, 1.0 / scaleFactor);
		}
	 
		//Get the position after scaling, in scene coords
		QPointF pointAfterScale(mapToScene(event->pos()));
	 
		//Get the offset of how the screen moved
		QPointF offset = pointBeforeScale - pointAfterScale;
	 
		//Adjust to the new center for correct zooming
		QPointF newCenter = screenCenter + offset;
		SetCenter(newCenter);
	}
}
 
/**
  * Need to update the center so there is no jolt in the
  * interaction after resizing the widget.
  */
void GraphicsView::resizeEvent (QResizeEvent * event)
{
	//Get the rectangle of the visible area in scene coords
	QRectF visibleArea = mapToScene(rect()).boundingRect();
	SetCenter(visibleArea.center());
 
	//Call the subclass resize so the scrollbars are updated correctly
	QGraphicsView::resizeEvent(event);
}

}

