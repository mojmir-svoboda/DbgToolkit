#include "profilerwindow.h"
#include <QtGui>
#include "profilerview.h"
#include "profilerbar.h"
#include "profilerblockinfo.h"

namespace profiler {

ProfilerWindow::ProfilerWindow(std::vector<ProfileInfo> & pis, QWidget * parent)
	: m_profileInfos(pis)
	, QWidget(parent)
{
	m_scene = new QGraphicsScene;
	populateScene();

	QSplitter * vSplitter = new QSplitter;
	vSplitter->setOrientation(Qt::Vertical);

	View * view = new View(this, "View 0");
	view->view()->setScene(m_scene);
	vSplitter->addWidget(view);

	//view = new View(this, "View 1");
	//view->view()->setScene(m_scene);
	//vSplitter->addWidget(view);

	QVBoxLayout * layout = new QVBoxLayout;
	layout->addWidget(vSplitter);
	setLayout(layout);

	setWindowTitle(tr("Profiler Demo"));
}

	struct HSV { float h; float s; float v; };
	inline float tmp_randf () { return (float)rand()/(float)RAND_MAX; }

	typedef std::map<std::string, QColor> colormap_t;
	colormap_t colors;

void ProfilerWindow::populateScene()
{
	printf("%s\n", __FUNCTION__);

	for (size_t p = 0, pe = m_profileInfos.size(); p < pe; ++p)
	{
		ProfileInfo & pi = m_profileInfos[p];

		//printf("p=%u\n", p); fflush(stdout);

		std::vector<unsigned> max_layers;

		for (size_t f = 0, fe = pi.m_completed_frame_infos.size(); f < fe; ++f)
		{
			threadinfos_t & tis = pi.m_completed_frame_infos[f];
			for (size_t t = 0, te = tis.size(); t < te; ++t)
			{
				max_layers.push_back(0);
				blockinfos_t & bis = tis[t];
				for (size_t b = 0, be = bis.size(); b < be; ++b)
				{
					BlockInfo & block = *bis[b];
					block.m_tag = block.m_msg;
					size_t const l = block.m_tag.find('[');
					size_t const r = block.m_tag.find(']');
					if (l != std::string::npos && r != std::string::npos)
						block.m_tag.erase(l, r - l);
					colors[block.m_tag] = Qt::gray;
					if (block.m_layer >= max_layers[t])
						max_layers[t] = block.m_layer;
				}
			}
		}

		//printf("color size: %u\n", colors.size()); fflush(stdout);
		if (colors.size() == 0)
			break;

		// pick colors for unique clusters
		std::vector<HSV> ucolors;
		ucolors.reserve(colors.size());
		for (size_t hi = 0; hi < 360; hi += 360 / colors.size())
		{
			HSV hsv;
			hsv.h = hi / 360.0f;
			hsv.s = 0.70f + tmp_randf() * 0.2f - 0.05f;
			hsv.v = 0.85f + tmp_randf() * 0.2f - 0.05f;
			ucolors.push_back(hsv);
		}

		int max_y = 0;
		int max_x = 0;

		for (size_t f = 0, fe = pi.m_completed_frame_infos.size(); f < fe; ++f)
		{
			threadinfos_t & tis = pi.m_completed_frame_infos[f];

			int const h = g_heightValue;
			int const space = g_spaceValue;
			unsigned offs = 1;
			for (size_t t = 0, te = tis.size(); t < te; ++t)
			{
				blockinfos_t & bis = tis[t];

				for (size_t b = 0, be = bis.size(); b < be; ++b)
				{
					BlockInfo & block = *bis[b];

					int w = block.m_dt;
					qreal x = block.m_time_bgn;
					qreal y = (offs) * (h + space)  + block.m_layer * (h + space);
					block.m_x = x / g_scaleValue;
					block.m_y = y;
					printf("f=%2u t=%2u b=%2u    (%3.2f, %3.2f) (x=%6.1f y=%6.1f w=%4i h=%4i dt=%3.3f)\n", f, t, b, block.m_x, block.m_y, x, y, w, h, block.m_dt); fflush(stdout);

					if (y > max_y)
						max_y = y;

					if (x > max_x)
						max_x = x;

					QColor color = Qt::white;
					colormap_t::iterator it = colors.find(block.m_tag);
					if (it != colors.end())
					{
						HSV hsv = ucolors[std::distance(colors.begin(), it)];
						color.setHsvF(hsv.h, hsv.s, hsv.v);
					}

					QGraphicsItem * item = new Bar(block, color, 0, 0, w, h, t, offs);
					item->setPos(QPointF(block.m_x, y));
					m_scene->addItem(item);
					item->setToolTip(QString("frame=%1 thread=%2 %3 [%4 ms]").arg(f).arg(t).arg(block.m_msg.c_str()).arg(block.m_dt / 1000.0f));

					QGraphicsItem * titem = new BarText(block, color, 0, 0, w, h, t, offs);
					titem->setPos(QPointF(block.m_x, y));
					m_scene->addItem(titem);
				}

				offs += max_layers[t] + 1;

				QPen p1;
				p1.setColor(Qt::gray);
				p1.setWidth(0);
				int y = (offs) * (h + space);
				QGraphicsLineItem * ln = new QGraphicsLineItem(0, y, max_x, y);
				ln->setPen(p1);
				m_scene->addItem(ln);

				offs += 1;
			}
		}

		for (size_t f = 0, fe = pi.m_completed_frame_infos.size(); f < fe; ++f)
		{
			threadinfos_t & tis = pi.m_completed_frame_infos[f];

			int const h = g_heightValue;
			int const space = g_spaceValue;
			for (size_t t = 0, te = tis.size(); t < te; ++t)
			{
				blockinfos_t & bis = tis[t];

				for (size_t b = 0, be = bis.size(); b < be; ++b)
				{
					BlockInfo & block = *bis[b];
					if (block.m_parent)
					{
						if (block.m_parent->m_x < 100.0f || block.m_parent->m_y < 100.0f)
						{
							// incomplete parent!
						}
						else
						{
							QPen p1;
							p1.setColor(Qt::blue);
							p1.setWidth(0);
							QGraphicsLineItem * ln_bg = new QGraphicsLineItem(block.m_x, block.m_y, block.m_parent->m_x, block.m_parent->m_y + g_heightValue);
							ln_bg->setPen(p1);
							m_scene->addItem(ln_bg);
							QGraphicsLineItem * ln_nd = new QGraphicsLineItem(block.m_x + block.m_dt, block.m_y, block.m_parent->m_x + block.m_parent->m_dt, block.m_parent->m_y + g_heightValue);
							p1.setColor(Qt::cyan);
							ln_nd->setPen(p1);
							m_scene->addItem(ln_nd);
						}
					}

					QPen p1;
					p1.setColor(Qt::gray);
					QGraphicsLineItem * ln_end = new QGraphicsLineItem(block.m_x + block.m_dt, block.m_y, block.m_x + block.m_dt, block.m_y + g_heightValue);
					ln_end->setPen(p1);
					p1.setWidth(4);
					m_scene->addItem(ln_end);
				}
			}
		}


		for (size_t f = 0, fe = pi.m_frames.size(); f < fe; ++f)
		{
			QGraphicsTextItem * txt = new QGraphicsTextItem(QString("%1").arg(f));
			txt->setFlag(QGraphicsItem::ItemIgnoresTransformations);
			txt->setPos(pi.m_frames[f].first, 7);
			m_scene->addItem(txt);

			QPen p1;
			p1.setColor(Qt::red);
			QGraphicsLineItem * line1 = new QGraphicsLineItem(pi.m_frames[f].first, 0, pi.m_frames[f].first, max_y);
			line1->setPen(p1);
			m_scene->addItem(line1);

			QPen p2;
			p2.setColor(Qt::yellow);
			QGraphicsLineItem * line2 = new QGraphicsLineItem(pi.m_frames[f].second, 0, pi.m_frames[f].second, max_y);
			line2->setPen(p2);
			m_scene->addItem(line2);
		}
	}
}

} // namespace profiler

