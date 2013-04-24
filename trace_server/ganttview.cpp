#include "ganttview.h"
#include <QtGui>
#include <QSpinBox>
#include <QSplitter>
#ifndef QT_NO_OPENGL
#	include <QtOpenGL>
#endif
#include <qmath.h>
#include "scalewidget.h"
#include "qwt/qwt_scale_widget.h"
#include "qwt/qwt_scale_draw.h"
#include "qwt/qwt_color_map.h"
#include "qwt/qwt_scale_engine.h"
#include "qwt/qwt_transform.h"
#include "ganttitem.h"
#include "arrow.h"
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
	std::random_shuffle(m_unique_colors.begin(), m_unique_colors.end());
}

GfxView & GanttView::createViewForContext (unsigned long long ctx, QGraphicsScene * s)
{
	contextviews_t::iterator it = m_contextviews.find(ctx);
	if (it == m_contextviews.end())		
	{
		GraphicsView * view = new GraphicsView(*this, m_gvcfg, this);
		connect(view->verticalScrollBar(), SIGNAL(valueChanged(int)), view, SLOT(verticalScroll(int)));
		connect(view->horizontalScrollBar(), SIGNAL(valueChanged(int)), view, SLOT(horizontalScroll(int)));
		view->setRenderHint(QPainter::Antialiasing, false);
		view->setDragMode(QGraphicsView::RubberBandDrag);
		view->setOptimizationFlags(QGraphicsView::DontSavePainterState);
		view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
		m_layout->addWidget(view);

		//view->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { width: 6px; }");
		view->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 10px; }");

		QGraphicsScene * scene = (s == 0) ? new QGraphicsScene() : s;
		GfxView g;
		g.m_view = view;
		g.m_scene = scene;
		g.m_view->setScene(g.m_scene);

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
	//qDebug("--{ +++ f=%i t=%8llu  ctxi=%u  msg=%s", m_ganttData.m_frame, m_ganttData.m_frame_begin, dd.m_ctx_idx, dd.m_text.toStdString().c_str());
	qDebug("{ FRAME %3i", m_ganttData.m_frame);
}

void GanttView::appendFrameEnd (DecodedData & dd)
{
	float const scale = m_gvcfg.m_timeunits;
	m_ganttData.m_frames.push_back(std::make_pair(m_ganttData.m_frame_begin * scale, dd.m_time * scale));

	//size_t const from = m_last_flush_end_idx;
	//size_t const to = m_ganttData.m_completed_frame_data.size();
	//qDebug("frame flushing from %i to %i", from, to);
	//qDebug("flushing from %i to %i", from, to);

	//for (size_t i = from; i < to; ++i)
	//{
		//qDebug("producing[%i], sz=%u", i, m_ganttData.m_completed_frame_data[i]->size());
		//consumeData(m_ganttData.m_completed_frame_data[i]);
	//}

	//m_last_flush_end_idx = to;

	qDebug("} FRAME=%3i", m_ganttData.m_frame);
	//qDebug("}-- --- f=%i t=%8llu  ctxi=%u  msg=%s", m_ganttData.m_frame, m_ganttData.m_frame_begin, dd.m_ctx_idx, dd.m_text.toStdString().c_str());
	//dump
	/*for (size_t i = from; i < to; ++i)
		for (size_t j = 0, je = m_ganttData.m_completed_frame_infos[i]->size(); j < je; ++j)
			qDebug("producing item[%i]=0x%016x, contexts=%u bis_sz=%u", i, m_ganttData.m_completed_frame_infos[i], m_ganttData.m_completed_frame_infos[i]->size(),  m_ganttData.m_completed_frame_infos[i]->operator[](j).size());
*/
}
void GanttView::appendBgn (DecodedData & dd)
{
	Data * prev = 0;
	if (m_ganttData.m_pending_data[dd.m_ctx_idx].size())
		prev = m_ganttData.m_pending_data[dd.m_ctx_idx].back();
	m_ganttData.m_data_ptrs.push_back(new Data());
	m_ganttData.m_pending_data[dd.m_ctx_idx].push_back(m_ganttData.m_data_ptrs.back());
	Data & d = *m_ganttData.m_pending_data[dd.m_ctx_idx].back();
	d.m_time_bgn_orig = dd.m_time;
	d.m_time_bgn = dd.m_time * m_gvcfg.m_timeunits;
	d.m_ctx = dd.m_ctx;
	d.m_ctx_idx = dd.m_ctx_idx;


	d.m_tag = dd.m_text;
	int const l = d.m_tag.indexOf('[');
	int const r = d.m_tag.indexOf(']');
	if (l != -1 && r != -1)
	{
		d.m_msg = dd.m_text.mid(l + 1, r - l - 1);
		d.m_tag.truncate(l);
		d.m_tag = d.m_tag.simplified();
	}

	d.m_layer = m_ganttData.m_pending_data[dd.m_ctx_idx].size() - 1;
	d.m_frame = m_ganttData.m_frame;
	d.m_parent = prev;

	qDebug("{ f=%i t=%8llu  ctxi=%u  tag='%s'  ['%s']", d.m_frame, d.m_time_bgn, d.m_ctx_idx, d.m_tag.toStdString().c_str(), d.m_msg.toStdString().c_str());
}

void GanttView::appendEnd (DecodedData & dd)
{
	unsigned const frame_idx = m_ganttData.m_frame;
	if (m_ganttData.m_pending_data[dd.m_ctx_idx].size())
	{
		Data * d = m_ganttData.m_pending_data[dd.m_ctx_idx].back();
		m_ganttData.m_pending_data[dd.m_ctx_idx].pop_back();

		d->m_time_end_orig = dd.m_time;
		d->m_time_end = dd.m_time * m_gvcfg.m_timeunits;
		d->m_endmsg = dd.m_text;
		d->complete(m_gvcfg.m_timeunits);

		(*m_ganttData.m_completed_frame_data[d->m_frame])[dd.m_ctx_idx].push_back(d);

		//qDebug("end flushing f=%i", d->m_frame);
		//consumeData(m_ganttData.m_completed_frame_data[d->m_frame]);
		consumeEnd(d);


		qDebug("} f=%i t=%8llu  ctxi=%u  tag='%s' ['%s']", d->m_frame, d->m_time_bgn, d->m_ctx_idx, d->m_tag.toStdString().c_str(), d->m_msg.toStdString().c_str());
	}
	else
		qWarning("Mismatched end! tag='%s' subtag='%s' text='%s'", dd.m_tag.toStdString().c_str(), dd.m_subtag.toStdString().c_str(), dd.m_text.toStdString().c_str());
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

void GanttView::consumeEnd (Data * end_data)
{
	if (end_data->m_frame >= m_ganttData.m_completed_frame_data.size())
	{
		qWarning("container not big enough");
		return;
	}

	contextdata_t & contexts = *m_ganttData.m_completed_frame_data[end_data->m_frame];
	
	unsigned const ci = end_data->m_ctx_idx;
	{
		GfxView & v = viewAt(ci);
		data_t const & datas = contexts[ci];
		//qDebug("processing data:, contexts_sz=%u datas_sz=%u", contexts.size(), datas.size());

		for (size_t di = 0, die = datas.size(); di < die; ++di)
		{
			if (end_data == datas[di])
			{
				Data & d = *datas[di];

				if (m_gvcfg.m_auto_color)
				{
					colormap_t::iterator it = m_tagcolors.find(d.m_tag);
					if (it == m_tagcolors.end())
					{
						//qDebug("new tag d_tag=\"%s\" d_msg=\"%s\"", d.m_tag.toStdString().c_str(), d.m_msg.toStdString().c_str());
						if (m_unique_colors.size() > 0)
						{
							d.m_color = m_unique_colors.back();
							m_unique_colors.pop_back();
						}
						//qDebug("COL= %s tag=%s", d.m_color.name().toStdString().c_str(), d.m_tag.toStdString().c_str());
						m_tagcolors[d.m_tag] = d.m_color;
					}
					else
						d.m_color = *it;
				}
				else
					d.m_color = Qt::white;

				if (d.m_layer >= m_max_layers[ci])
					m_max_layers[ci] = d.m_layer;
			}
		}

		int max_y = 0;
		int max_x = 0;

		int const h = g_heightValue;
		int const space = g_spaceValue;
		unsigned offs = 1;

		for (size_t di = 0, die = datas.size(); di < die; ++di)
		{
			if (end_data == datas[di])
			{
				Data & d = *datas[di];
				int w = d.m_dt;
				qreal x = d.m_time_bgn;
				qreal y = (offs) * (h + space)  + d.m_layer * (h + space);
				d.m_x = x;
				d.m_y = y;
				qDebug("++++ f=%2u ci=%2u di=%2u  %s  (x=%6.1f y=%6.1f w=%4i h=%4i dt=%3.1f) col=%s", d.m_frame, ci, di, d.m_tag.toStdString().c_str(), x, y, w, h, d.m_dt, d.m_color.name().toStdString().c_str());

				if (y > max_y)
					max_y = y;

				if (x > max_x)
					max_x = x;

				QGraphicsItem * item = new BarItem(m_gvcfg, d, d.m_color, 0, 0, w, h, ci, offs);
				item->setPos(QPointF(d.m_x, y));
				item->setToolTip(QString("timing=<%1..%2>\nframe=%3 ctx=%4\n%5 %6\n%7\n[%8 ms]").arg(d.m_time_bgn).arg(d.m_time_end).arg(d.m_frame).arg(ci).arg(d.m_tag).arg(d.m_msg).arg(d.m_endmsg).arg(d.m_dt));
				d.m_item = item;

				QGraphicsItem * titem = new BarTextItem(item, m_gvcfg, d, d.m_color, 0, 0, w, h, ci, offs);
				titem->setPos(QPointF(0, 0));
				d.m_textitem = titem;

				v.m_scene->addItem(item);
			}
		}

		offs += m_max_layers[ci];
	}

	int const h = g_heightValue;
	int const space = g_spaceValue;

	{
		GfxView & v = viewAt(ci);
		data_t const & datas = contexts[ci];

		for (size_t di = 0, be = datas.size(); di < be; ++di)
		{
			if (end_data == datas[di])
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
						QPointF bg_p0(d.m_x, d.m_parent->m_y + g_heightValue);
						QPointF bg_p1(d.m_x, d.m_y);
						Arrow * arrow_bg = new Arrow(d.m_parent->m_item, d.m_item, bg_p0, bg_p1, 0, 0);
						v.m_scene->addItem(arrow_bg);

						QPointF nd_p0(d.m_x + d.m_dt, d.m_y);
						QPointF nd_p1(d.m_parent->m_x + d.m_parent->m_dt, d.m_parent->m_y + g_heightValue);
						Arrow * arrow_nd = new Arrow(d.m_parent->m_item, d.m_item, nd_p0, nd_p1, 0, 0);
						v.m_scene->addItem(arrow_nd);


						//QGraphicsLineItem * ln_bg = new QGraphicsLineItem(d.m_x, d.m_parent->m_y + g_heightValue, d.m_x, d.m_y);
						//ln_bg->setPen(p1);
						//v.m_scene->addItem(ln_bg);
						//QGraphicsLineItem * ln_nd = new QGraphicsLineItem(d.m_x + d.m_dt, d.m_y, d.m_parent->m_x + d.m_parent->m_dt, d.m_parent->m_y + g_heightValue);
						//p1.setColor(Qt::black);
						//ln_nd->setPen(p1);
						//v.m_scene->addItem(ln_nd);
					}
				}

				QPen p1;
				p1.setColor(Qt::black);
				p1.setWidth(0);

				QGraphicsLineItem * ln_bgn = new QGraphicsLineItem(d.m_x, d.m_y, d.m_x, d.m_y + g_heightValue);
				ln_bgn->setPen(p1);
				v.m_scene->addItem(ln_bgn);

				QGraphicsLineItem * ln_end = new QGraphicsLineItem(d.m_x + d.m_dt, d.m_y, d.m_x + d.m_dt, d.m_y + g_heightValue);
				ln_end->setPen(p1);
				v.m_scene->addItem(ln_end);
			}
		}
	} 
}

void GanttView::consumeData (contextdata_t * c)
{
	contextdata_t & contexts = *c;
	//qDebug("+++ DATA consumed node: contexts_sz=%u", contexts.size());
	for (size_t ci = 0, te = contexts.size(); ci < te; ++ci)
	{
		GfxView & v = viewAt(ci);
		data_t const & datas = contexts[ci];
		//qDebug("processing data:, contexts_sz=%u datas_sz=%u", contexts.size(), datas.size());
		for (size_t di = 0, die = datas.size(); di < die; ++di)
		{
			Data & d = *datas[di];

			if (m_gvcfg.m_auto_color)
			{
				colormap_t::iterator it = m_tagcolors.find(d.m_tag);
				if (it == m_tagcolors.end())
				{
					//qDebug("new tag d_tag=\"%s\" d_msg=\"%s\"", d.m_tag.toStdString().c_str(), d.m_msg.toStdString().c_str());
					if (m_unique_colors.size() > 0)
					{
						d.m_color = m_unique_colors.back();
						m_unique_colors.pop_back();
					}
					//qDebug("COL= %s tag=%s", d.m_color.name().toStdString().c_str(), d.m_tag.toStdString().c_str());
					m_tagcolors[d.m_tag] = d.m_color;
				}
				else
					d.m_color = *it;
			}
			else
				d.m_color = Qt::white;

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

			int w = d.m_dt;
			qreal x = d.m_time_bgn;
			qreal y = (offs) * (h + space)  + d.m_layer * (h + space);
			d.m_x = x;
			d.m_y = y;
			qDebug("++++ f=%2u ci=%2u di=%2u  %s  (x=%6.1f y=%6.1f w=%4i h=%4i dt=%3.1f) col=%s", d.m_frame, ci, di, d.m_tag.toStdString().c_str(), x, y, w, h, d.m_dt, d.m_color.name().toStdString().c_str());

			if (y > max_y)
				max_y = y;

			if (x > max_x)
				max_x = x;

			QGraphicsItem * item = new BarItem(m_gvcfg, d, d.m_color, 0, 0, w, h, ci, offs);
			item->setPos(QPointF(d.m_x, y));
			item->setToolTip(QString("timing=<%1..%2>\nframe=%3 ctx=%4\n%5 %6\n%7\n[%8 ms]").arg(d.m_time_bgn).arg(d.m_time_end).arg(d.m_frame).arg(ci).arg(d.m_tag).arg(d.m_msg).arg(d.m_endmsg).arg(d.m_dt));
			d.m_item = item;

			QGraphicsItem * titem = new BarTextItem(item, m_gvcfg, d, d.m_color, 0, 0, w, h, ci, offs);
			titem->setPos(QPointF(0, 0));
			d.m_textitem = titem;

			v.m_scene->addItem(item);
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
					QPointF bg_p0(d.m_x, d.m_parent->m_y + g_heightValue);
					QPointF bg_p1(d.m_x, d.m_y);
					Arrow * arrow_bg = new Arrow(d.m_parent->m_item, d.m_item, bg_p0, bg_p1, 0, 0);
					v.m_scene->addItem(arrow_bg);

					QPointF nd_p0(d.m_x + d.m_dt, d.m_y);
					QPointF nd_p1(d.m_parent->m_x + d.m_parent->m_dt, d.m_parent->m_y + g_heightValue);
					Arrow * arrow_nd = new Arrow(d.m_parent->m_item, d.m_item, nd_p0, nd_p1, 0, 0);
					v.m_scene->addItem(arrow_nd);


					//QGraphicsLineItem * ln_bg = new QGraphicsLineItem(d.m_x, d.m_parent->m_y + g_heightValue, d.m_x, d.m_y);
					//ln_bg->setPen(p1);
					//v.m_scene->addItem(ln_bg);
					//QGraphicsLineItem * ln_nd = new QGraphicsLineItem(d.m_x + d.m_dt, d.m_y, d.m_parent->m_x + d.m_parent->m_dt, d.m_parent->m_y + g_heightValue);
					//p1.setColor(Qt::black);
					//ln_nd->setPen(p1);
					//v.m_scene->addItem(ln_nd);
				}
			}

			QPen p1;
			p1.setColor(Qt::black);
			p1.setWidth(0);

			QGraphicsLineItem * ln_bgn = new QGraphicsLineItem(d.m_x, d.m_y, d.m_x, d.m_y + g_heightValue);
			ln_bgn->setPen(p1);
			v.m_scene->addItem(ln_bgn);

			QGraphicsLineItem * ln_end = new QGraphicsLineItem(d.m_x + d.m_dt, d.m_y, d.m_x + d.m_dt, d.m_y + g_heightValue);
			ln_end->setPen(p1);
			v.m_scene->addItem(ln_end);
		}
	} 
}


void GanttView::applyConfig (GanttViewConfig & gvcfg)
{
	for (contextviews_t::iterator it = m_contextviews.begin(), ite = m_contextviews.end(); it != ite; ++it)
	{
		// @TODO: only the first one, the others will be synced
		updateTimeWidget((*it).m_view);
		break;
	}

	if (m_curr_strtime_units != gvcfg.m_strtimeunits)
	{
		for (contextviews_t::iterator it = m_contextviews.begin(), ite = m_contextviews.end(); it != ite; ++it)
		{
			it->m_scene->clear();

			double scale = m_curr_timeunits / gvcfg.m_timeunits;
			qDebug("scaling %f", scale);
			for (size_t f = 0, fe = m_ganttData.m_completed_frame_data.size(); f < fe; ++f)
				for (size_t c = 0, ce = m_ganttData.m_completed_frame_data[f]->size(); c < ce; ++c)
					for (size_t i = 0, ie = m_ganttData.m_completed_frame_data[f][c].size(); i < ie; ++i)
					{
						Data * d = (*m_ganttData.m_completed_frame_data[f])[c][i];
						d->scale(scale);
						BarItem * item = static_cast<BarItem *>(d->m_item);
						it->m_scene->addItem(item);
					}
		}
		m_curr_strtime_units = gvcfg.m_strtimeunits;
	}
}

void GanttView::updateTimeWidget (GraphicsView * v)
{
    //QPointF tl(v->horizontalScrollBar()->value(), v->verticalScrollBar()->value());
	//QPointF br = tl + v->viewport()->rect().bottomRight();
	//QMatrix invmat = v->matrix().inverted();
	//QRectF rr = invmat.mapRect(QRectF(tl,br));

	QRectF const vscenerect = v->mapToScene(v->rect()).boundingRect();
	QPointF tl1 = vscenerect.topLeft();
	QPointF br1 = vscenerect.bottomRight();

	QwtLinearScaleEngine se;
	QwtTransform * t = se.transformation();
	QwtInterval interval(tl1.x(), br1.x());
	QwtScaleDiv d = se.divideScale(interval.minValue(), interval.maxValue(), 20, 20);
	m_timewidget->setScaleDiv(d); // as in QwtPlot::Axis
	m_timewidget->setColorBarEnabled(true);
	//m_timewidget->setColorMap(interval, colormap);
	m_timewidget->setToolTip(QString("time [%1]").arg(m_gvcfg.m_strtimeunits));
	//m_timewidget->setMargin(2);

	// layout to adjust scale
	//int const margin = m_timewidget->scaleMargin();
	//m_timewidget->setMinimumSize(QSize(0, 27 - 2 * margin));
}


GanttView::GanttView (Connection * conn, QWidget * parent, gantt::GanttViewConfig & config, QString const & fname)
	: QFrame(parent)
	, m_connection(conn)
	, m_curr_timeunits(1.0f)
	, m_gvcfg(config)
	, m_last_flush_end_idx(0)
	, m_timewidget(0)
{
	qDebug("%s", __FUNCTION__);

	setFrameStyle(Sunken | StyledPanel);
	initColors();

	m_timewidget = new ScaleWidget(QwtScaleDraw::TopScale, this);

	m_layout = new QSplitter(Qt::Vertical);
	m_layout->setContentsMargins(QMargins(0, 0, 0, 0));
	QGridLayout * grid = new QGridLayout(this);
	grid->setContentsMargins(QMargins(0, 0, 0, 0));
	grid->addWidget(m_layout, 0, 0);
	grid->setVerticalSpacing(0);
	grid->setHorizontalSpacing(0);

	// Qwt scale widget and stuff
	m_layout->addWidget(m_timewidget);
	setLayout(grid);

	config.setTimeUnits(config.m_strtimeunits);
	m_curr_strtime_units = config.m_strtimeunits;
	m_curr_timeunits = config.m_timeunits;

	//connect(m_resetButton, SIGNAL(clicked()), this, SLOT(resetView()));
	//connect(m_zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
	//connect(m_antialiasButton, SIGNAL(toggled(bool)), this, SLOT(toggleAntialiasing()));
	//connect(m_openGlButton, SIGNAL(toggled(bool)), this, SLOT(toggleOpenGL()));
	//connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
	//connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));

	setupMatrix();
	m_ganttData.m_completed_frame_data.push_back(new contextdata_t()); // @TODO: reserve
}

void GanttView::resetView()
{
	//m_zoomSlider->setValue(250);
	setupMatrix();

	for (contextviews_t::iterator it = m_contextviews.begin(), ite = m_contextviews.end(); it != ite; ++it)
		(*it).m_view->ensureVisible(QRectF(0, 0, 0, 0));
}

void GanttView::setupMatrix ()
{
	qreal scale = qPow(qreal(2), (1.0f - 1.0f) / qreal(50));

	QMatrix matrix;
	if (m_gvcfg.m_y_scaling)
	{
		matrix.scale(scale, scale);
	}
	else
	{
		matrix.scale(scale, 1.0f);
	}
	m_gvcfg.m_scale = scale;

	for (contextviews_t::iterator it = m_contextviews.begin(), ite = m_contextviews.end(); it != ite; ++it)
		(*it).m_view->setMatrix(matrix);
}

void GanttView::toggleOpenGL ()
{
#ifndef QT_NO_OPENGL
	//m_graphicsView->setViewport(m_openGlButton->isChecked() ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
#endif
}

void GanttView::toggleAntialiasing ()
{
	//m_graphicsView->setRenderHint(QPainter::Antialiasing, m_antialiasButton->isChecked());
}

void GanttView::zoomIn()
{
	m_gvcfg.m_scale += 1.0f;
}

void GanttView::zoomOut()
{
	m_gvcfg.m_scale -= 1.0f;
}

void GanttView::forceUpdate ()
{
	for (contextviews_t::iterator it = m_contextviews.begin(), ite = m_contextviews.end(); it != ite; ++it)
		(*it).m_view->viewport()->update();
}

GraphicsView::GraphicsView (GanttView & gv, GanttViewConfig & gvcfg, QWidget * parent)
	: QGraphicsView(parent)
	, m_gv(gv)
    , m_gvcfg(gvcfg)
{ }

void GraphicsView::verticalScroll (int n)
{
	m_gv.updateTimeWidget(this);
}

void GraphicsView::horizontalScroll (int n)
{
	m_gv.updateTimeWidget(this);
}

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
	QRectF const visibleArea = mapToScene(rect()).boundingRect();
	// Get the scene area
	QRectF const sceneBounds = sceneRect();

	double const boundX = visibleArea.width() / 2.0;
	double const boundY = visibleArea.height() / 2.0;
	double const boundWidth = sceneBounds.width() - 2.0 * boundX;
	double const boundHeight = sceneBounds.height() - 2.0 * boundY;

	//qDebug("setcenter: x=%f y=%f w=%f h=%f", boundX, boundY, boundWidth, boundHeight);
	// The max boundary that the centerPoint can be to
	QRectF const bounds(boundX, boundY, boundWidth, boundHeight);
	if (bounds.contains(centerPoint))
	{
		// We are within the bounds
		m_current_center = centerPoint;
	}
	else
	{
		// We need to clamp or use the center of the screen
		if (visibleArea.contains(sceneBounds))
		{
			// Use the center of scene ie. we can see the whole scene
			m_current_center = centerPoint;
			//m_current_center = sceneBounds.center();
		}
		else
		{
			m_current_center = centerPoint;
 
			//We need to clamp the center. The centerPoint is too large
			if (centerPoint.x() > bounds.x() + bounds.width()) {
				m_current_center.setX(bounds.x() + bounds.width());
			} else if (centerPoint.x() < bounds.x()) {
				m_current_center.setX(bounds.x());
			}
 
			if (centerPoint.y() > bounds.y() + bounds.height()) {
				m_current_center.setY(bounds.y() + bounds.height());
			} else if (centerPoint.y() < bounds.y()) {
				m_current_center.setY(bounds.y());
			}
		}
	}
	// Update the scrollbars
	centerOn(m_current_center);
	//qDebug("setcenter: curr=%f y=%f", boundX, boundY, boundWidth, boundHeight);

	m_gv.updateTimeWidget(this);
}
 
void GraphicsView::mousePressEvent (QMouseEvent * event)
{
	LastPanPoint = event->pos(); // For panning the view
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
		// get how much user panned
		QPointF const delta = mapToScene(LastPanPoint) - mapToScene(event->pos());
		LastPanPoint = event->pos();

		QPointF const cen = mapToScene(viewport()->rect()).boundingRect().center();
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
 
void GraphicsView::wheelEvent (QWheelEvent* event)
{
	bool const shift = event->modifiers() & Qt::SHIFT;

	if (shift)
	{
		//m_frameSpinBox.setValue(m_frameSpinBox.value() + event->delta());
	}
	else
	{
		//qDebug("wheelEvent: x=%i y=%i", event->pos().x(), event->pos().y());
		QPointF pointBeforeScale(mapToScene(event->pos())); // get the position of the mouse before scaling, in scene coords
		// get the original screen centerpoint
		//QPointF const screenCenter = GetCenter(); //m_current_center; //(visRect.center());
		SetCenter(pointBeforeScale);
		QPointF const screenCenter = m_current_center; //(visRect.center());

		double const scaleFactor = 1.15; // how fast we zoom
		double const scaleYFactor = (m_gvcfg.m_y_scaling) ? scaleFactor : 1.0f;
		if (event->delta() > 0)
			scale(scaleFactor, scaleYFactor); // zoom in
		else
			scale(1.0 / scaleFactor, 1.0 / scaleYFactor); // zooming out
		
		QPointF const pointAfterScale(mapToScene(event->pos())); // position after scaling, in scene coords
		QPointF const offset = pointBeforeScale - pointAfterScale; // offset of how the screen moved
		QPointF const newCenter = screenCenter + offset; // adjust to the new center for correct zooming
		SetCenter(newCenter);

		m_gv.updateTimeWidget(this);
	}
}
 
/**
  * Need to update the center so there is no jolt in the
  * interaction after resizing the widget.
  */
void GraphicsView::resizeEvent (QResizeEvent * event)
{
	QRectF const visibleArea = mapToScene(rect()).boundingRect();//Get the rectangle of the visible area in scene coords
	SetCenter(visibleArea.center());
	QGraphicsView::resizeEvent(event);

	//@FIXME: onAfterResizeEvent or something like that
	//static_cast<GanttView *>(parent())->updateTimeWidget(this);
}

}

