#include "mainwindow.h"
#include "view.h"
#include "bar.h"
#include "blockinfo.h"

#include <QtGui>

MainWindow::MainWindow(std::vector<ProfileInfo> & pis, QWidget * parent)
	: m_profileInfos(pis)
	, QWidget(parent)
{
	populateScene();

	QSplitter * vSplitter = new QSplitter;
	vSplitter->setOrientation(Qt::Vertical);

	View * view = new View("View 0");
	view->view()->setScene(m_scene);
	vSplitter->addWidget(view);

	view = new View("View 1");
	view->view()->setScene(m_scene);
	vSplitter->addWidget(view);

	QVBoxLayout * layout = new QVBoxLayout;
	layout->addWidget(vSplitter);
	setLayout(layout);

	setWindowTitle(tr("Profiler Demo"));
}

void MainWindow::populateScene()
{
	m_scene = new QGraphicsScene;

	for (size_t p = 0, pe = m_profileInfos.size(); p < pe; ++p)
	{
		ProfileInfo & pi = m_profileInfos[p];

		for (size_t f = 0, fe = pi.m_completed_frame_infos.size(); f < fe; ++f)
		{
			threadinfos_t & tis = pi.m_completed_frame_infos[f];

			for (size_t t = 0, te = tis.size(); t < te; ++t)
			{
				blockinfos_t & bis = tis[t];

				for (size_t b = 0, be = bis.size(); b < be; ++b)
				{
					BlockInfo & block = bis[b];

					int w = block.m_delta_t;
					int h = 25;
					qreal x = block.m_time_bgn;
					qreal y = (t + 2) * block.m_layer * 30;
					//printf("f=%2u t=%2u b=%2u    x=%6.1f y=%6.1f w=%4i h=%4i\n", f, t, b, x, y, w, h); fflush(stdout);

					QColor color((t + 1) * 50, f * 30 + 60, b * 40 + 50);
					QGraphicsItem * item = new Bar(block, color, 0, 0, w, h);
					item->setPos(QPointF(x, y));
					m_scene->addItem(item);
				}
			}
		}

		for (size_t f = 0, fe = pi.m_frames.size(); f < fe; ++f)
		{
			QGraphicsLineItem * line1 = new QGraphicsLineItem(pi.m_frames[f].first, 0, pi.m_frames[f].first, 500);
			m_scene->addItem(line1);

			QGraphicsLineItem * line2 = new QGraphicsLineItem(pi.m_frames[f].second, 0, pi.m_frames[f].second, 500);
			m_scene->addItem(line2);
		}
	}
}
