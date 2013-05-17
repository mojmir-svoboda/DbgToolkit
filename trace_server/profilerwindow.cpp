#include "profilerwindow.h"
#include "profilerconnection.h"
#include <QtGui>
#include <QMainWindow>
#include "profilerview.h"
#include "profilerbar.h"
#include "profilermainwindow.h"
#include "ui_profilermainwindow.h"
#include "profilerblockinfo.h"
#include "hsv.h"
#include "utils.h"
#include "dock.h"

namespace profiler {

using namespace plot;

ProfilerWindow::ProfilerWindow (QMainWindow * window, QObject * parent, profiler::profiler_rvp_t * rvp)
	: QObject(parent)
	, m_window(window)
	, m_tag_model(0)
	, m_rvp(0)
	, m_tagWidget(0)
{
	qDebug("%s", __FUNCTION__);
}

ProfilerWindow::~ProfilerWindow ()
{
	disconnect(m_rvp->m_Source.get(), SIGNAL(incomingProfilerData(profiler::profiler_rvp_t *)), this, SLOT(incomingProfilerData(profiler::profiler_rvp_t *)));
	qDebug("%s", __FUNCTION__);
	delete m_rvp;
}

void ProfilerWindow::registerTag (BlockInfo const & bi)
{
	QString s;
	s.reserve(256);
	static std::vector<QString> tmp; // @TODO: hey piggy
	tmp.reserve(32);
	tmp.clear();

	tmp.push_back(bi.m_tag);
	BlockInfo * parent = bi.m_parent;
	while (parent)
	{
		tmp.push_back(parent->m_tag);
		parent = parent->m_parent;
	}

	for (std::vector<QString>::const_reverse_iterator it=tmp.rbegin(), ite=tmp.rend(); it != ite; ++it)
		s += QString("/") + (*it);

	//appendToTagTree(s);
	//sessionState().m_tag_filters.set_to_state();
}

GfxView ProfilerWindow::mkGfxView (int i)
{
	GfxView tmp;
	tmp.m_scene = new QGraphicsScene();
	tmp.m_view = new View(this, "View 0");
	tmp.m_view->view()->setScene(tmp.m_scene);
	tmp.m_wd = m_docks.mkDockWidget(m_window, tmp.m_view, true, QString("prof_detail"));
	return tmp;
}

GfxView & ProfilerWindow::viewAt (size_t i)
{
	if (i >= m_views.size())
	{
		m_views.push_back(GfxView());
	}

	if (m_views[i].m_scene == 0)
	{
		m_views[i] = mkGfxView(i);
	}
	return m_views[i];
}

void ProfilerWindow::incomingProfilerData (profiler::profiler_rvp_t * rvp)
{
	/*qDebug("%s", __FUNCTION__);
	threadinfos_t * node = 0;
	while (rvp->consume(node))
	{
		//qDebug("consumed node: 0x%016x", node);
		threadinfos_t const & tis = *node;

		qDebug("consumed node: 0x%016x, tis_sz=%u", node, tis.size());
		for (size_t t = 0, te = tis.size(); t < te; ++t)
		{
			GfxView & v = viewAt(t);
		...
	 */
}

} // namespace profiler

