#include "profilerwindow.h"
#include "profilerconnection.h"
#include <QtGui>
#include <QMainWindow>
#include "profilerview.h"
#include "profilerbar.h"
#include "profilermainwindow.h"
#include "simpleplot.h"
#include "pickableplot.h"
#include "ui_profilermainwindow.h"
#include "profilerblockinfo.h"
#include "hsv.h"
#include "utils.h"
#include "dock.h"

namespace profiler {

using namespace plot;

ProfilerWindow::ProfilerWindow (QObject * parent, profiler::profiler_rvp_t * rvp)
	: QObject(parent)
	, m_window(0)
	, m_scene(0)
	, m_rvp(0)
	, m_tagWidget(0)
{
	qDebug("%s", __FUNCTION__);
	m_window = new ProfilerMainWindow();
	
	m_scene = new QGraphicsScene();
	m_view = new View(this, "View 0");
	m_view->view()->setScene(m_scene);
	mkDockWidget(m_window, m_view, QString("detail"));

	PickablePlot * plot = new PickablePlot();
	mkDockWidget(m_window, plot, QString("frames"));

	m_tagWidget = new QTreeView();
	m_tagWidget->setModel(new QStandardItemModel);
	connect(m_tagWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtTagTree(QModelIndex)));
	QObject::connect(m_tagWidget, SIGNAL(expanded(QModelIndex const &)), this, SLOT(onFileExpanded(QModelIndex const &)));
	QObject::connect(m_tagWidget, SIGNAL(collapsed(QModelIndex const &)), this, SLOT(onFileCollapsed(QModelIndex const &)));
	mkDockWidget(m_window, m_tagWidget, QString("tags"));

	m_window->setEnabled(true);
	m_window->show();
	m_window->setWindowTitle(tr("Profiler Demo"));

	connect(rvp->m_Source.get(), SIGNAL(incomingProfilerData(profiler::profiler_rvp_t *)), this, SLOT(incomingProfilerData(profiler::profiler_rvp_t *)), Qt::QueuedConnection);

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

ProfilerWindow::~ProfilerWindow ()
{
	disconnect(m_rvp->m_Source.get(), SIGNAL(incomingProfilerData(profiler::profiler_rvp_t *)), this, SLOT(incomingProfilerData(profiler::profiler_rvp_t *)));
	qDebug("%s", __FUNCTION__);
	delete m_rvp;
}

	std::vector<QString> s;	// @TODO: hey piggy, to member variables

void ProfilerWindow::onClickedAtTagTree (QModelIndex idx)
{
	QStandardItemModel const * const model = static_cast<QStandardItemModel *>(m_tagWidget->model());
	QStandardItem * const node = model->itemFromIndex(idx);

	s.clear();
	s.reserve(16);
	s.push_back(model->data(idx, Qt::DisplayRole).toString());
	QStandardItem * parent = node->parent();
	std::string tagpath;
	QModelIndex parent_idx = model->indexFromItem(parent);
	while (parent_idx.isValid())
	{
		QString const & val = model->data(parent_idx, Qt::DisplayRole).toString();
		s.push_back(val);
		parent = parent->parent();
		parent_idx = model->indexFromItem(parent);
	}

	for (std::vector<QString>::const_reverse_iterator it=s.rbegin(), ite=s.rend(); it != ite; ++it)
		tagpath += std::string("/") + (*it).toStdString();

	Qt::CheckState const curr_state = node->checkState();
	if (curr_state == Qt::Checked)
	{
		// unchecked --> checked
		setCheckStateChilds(node, curr_state);
		sessionState().m_tag_filters.set_state_to_childs(tagpath, static_cast<E_NodeStates>(curr_state));

		QStandardItem * p = node;
		while (p = p->parent())
		{
			bool const all_checked = checkChildState(p, Qt::Checked);
			if (all_checked)
			{
				p->setCheckState(Qt::Checked);
			}
		}
	}
	else if (curr_state == Qt::Unchecked)
	{
		// checked --> unchecked
		set_state_to_topdown(sessionState().m_tag_filters, tagpath, static_cast<E_NodeStates>(curr_state), e_PartialCheck);
		setCheckStateChilds(node, curr_state);
		setCheckStateReverse(node->parent(), Qt::PartiallyChecked); // iff parent unchecked and clicked on leaf
	}

	E_NodeStates const new_state = static_cast<E_NodeStates>(curr_state);

	qDebug("tag click! sync state of %s --> node_checkstate=%i", tagpath.c_str(), node->checkState());
	sessionState().m_tag_filters.set_to_state(tagpath, static_cast<E_NodeStates>(new_state));
	//conn->onInvalidateFilter();
}

void ProfilerWindow::loadState ()
{
	//loadSessionState(conn->sessionState(), m_session_state);
}

	boost::char_separator<char> sep(":/\\");

void ProfilerWindow::appendToTagTree (std::string const & tagpath)
{
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
	tokenizer_t tok(tagpath, sep);

	QStandardItemModel * model = static_cast<QStandardItemModel *>(m_tagWidget->model());
	QStandardItem * node = model->invisibleRootItem();
	QStandardItem * last_hidden_node = 0;
	bool stop = false;
	std::string path;
	tokenizer_t::const_iterator it = tok.begin(), ite = tok.end();
	while (it != ite)
	{
		QString qItem = QString::fromStdString(*it);
		QStandardItem * child = findChildByText(node, qItem);
		path += "/";
		path += *it;
		if (child != 0)
		{
			node = child;

			E_NodeStates ff_state = e_Unchecked;
			bool col_state = true;
			TagInfo const * fi = 0;
			bool const known = sessionState().m_tag_filters.is_present(path, fi);
			if (known)
			{
				node->setCheckState(static_cast<Qt::CheckState>(fi->m_state));
				m_tagWidget->setExpanded(model->indexFromItem(node), !fi->m_collapsed);
			}

			if (!stop)
			{
				if (child->rowCount() == 1)
				{
					last_hidden_node = node;
				}
				else if (child->rowCount() > 1)
				{
					stop = true;
					last_hidden_node = node;
				}
			}
		}
		else
		{
			stop = true;

			E_NodeStates new_state = e_Checked;
			E_NodeStates ff_state = e_Unchecked;
			TagInfo ff;
			bool const known = sessionState().m_tag_filters.is_present(path, ff);
			if (known)
			{
				node->setCheckState(static_cast<Qt::CheckState>(ff.m_state));
				new_state = ff_state;
			}
			else
			{
				new_state = e_Checked;
				qDebug("unknown: %s, state=%u", path.c_str(), new_state);
				sessionState().m_tag_filters.set_to_state(tagpath, static_cast<E_NodeStates>(new_state));
			}

			bool const orig_exp = m_tagWidget->isExpanded(model->indexFromItem(node));
			qDebug("new node: %s, state=%u", path.c_str(), new_state);
			QList<QStandardItem *> row_items = addRowTriState(qItem, new_state);
			node->appendRow(row_items);

			if (ff_state == e_PartialCheck)
			{
				m_tagWidget->setExpanded(model->indexFromItem(node), orig_exp);
			}

			node = row_items.at(0);
		}
		++it;

		if (it == ite)
		{
			Qt::CheckState const curr_state = node->checkState();
			if (curr_state == Qt::Checked)
			{
				// unchecked --> checked
				setCheckStateChilds(node, curr_state);
				sessionState().m_tag_filters.set_state_to_childs(tagpath, static_cast<E_NodeStates>(curr_state));

				if (node->parent())
				{
					//@TODO progress up the tree (reverse)
					bool const all_checked = checkChildState(node->parent(), Qt::Checked);
					if (all_checked && node->parent())
						node->parent()->setCheckState(Qt::Checked);
				}
			}
			else if (curr_state == Qt::Unchecked)
			{
				// checked --> unchecked
				set_state_to_topdown(sessionState().m_tag_filters, tagpath, static_cast<E_NodeStates>(curr_state), e_PartialCheck);
				setCheckStateChilds(node, curr_state);
				setCheckStateReverse(node->parent(), Qt::PartiallyChecked); // iff parent unchecked and clicked on leaf
			}

			E_NodeStates const new_state = static_cast<E_NodeStates>(curr_state);

			qDebug("file click! sync state of %s --> node_checkstate=%i", tagpath.c_str(), node->checkState());
			sessionState().m_tag_filters.set_to_state(tagpath, static_cast<E_NodeStates>(new_state));

		}
	}
	/*if (last_hidden_node)
	{
		if (last_hidden_node->parent())
			m_main_window->getWidgetFile()->setRootIndex(last_hidden_node->parent()->index());
		else
			m_main_window->getWidgetFile()->setRootIndex(last_hidden_node->index());
	}*/
}


void ProfilerWindow::registerTag (BlockInfo const & bi)
{
	std::string s;
	s.reserve(256);
	static std::vector<std::string> tmp; // @TODO: hey piggy
	tmp.reserve(32);
	tmp.clear();

	tmp.push_back(bi.m_tag);
	BlockInfo * parent = bi.m_parent;
	while (parent)
	{
		tmp.push_back(parent->m_tag);
		parent = parent->m_parent;
	}

	for (std::vector<std::string>::const_reverse_iterator it=tmp.rbegin(), ite=tmp.rend(); it != ite; ++it)
		s += std::string("/") + (*it);

	appendToTagTree(s);
	//sessionState().m_tag_filters.set_to_state();
}

void ProfilerWindow::onFileColOrExp (QModelIndex const & idx, bool collapsed)
{
	QStandardItemModel const * const model = static_cast<QStandardItemModel *>(m_tagWidget->model());
	QStandardItem * const node = model->itemFromIndex(idx);

	std::vector<QString> s;	// @TODO: hey piggy, to member variables
	s.clear();
	s.reserve(16);
	QStandardItem * parent = node;
	QModelIndex parent_idx = model->indexFromItem(parent);
	while (parent_idx.isValid())
	{
		QString const & val = model->data(parent_idx, Qt::DisplayRole).toString();
		s.push_back(val);
		parent = parent->parent();
		parent_idx = model->indexFromItem(parent);
	}

	std::string file;
	for (std::vector<QString>::const_reverse_iterator it=s.rbegin(), ite=s.rend(); it != ite; ++it)
		file += std::string("/") + (*it).toStdString();

	sessionState().m_tag_filters.set_to_state(file, TagInfo(static_cast<E_NodeStates>(node->checkState()), collapsed));
}

void ProfilerWindow::onFileExpanded (QModelIndex const & idx)
{
	onFileColOrExp(idx, false);
}

void ProfilerWindow::onFileCollapsed (QModelIndex const & idx)
{
	onFileColOrExp(idx, true);
}

void ProfilerWindow::incomingProfilerData (profiler::profiler_rvp_t * rvp)
{
	qDebug("%s", __FUNCTION__);
	threadinfos_t * node = 0;
	while (rvp->consume(node))
	{
		//qDebug("consumed node: 0x%016x", node);
		threadinfos_t const & tis = *node;

		qDebug("consumed node: 0x%016x, tis_sz=%u", node, tis.size());
		for (size_t t = 0, te = tis.size(); t < te; ++t)
		{
			blockinfos_t const & bis = tis[t];
			qDebug("processing bi node: 0x%016x, tis_sz=%u bis_sz=%u", node, tis.size(), bis.size());
			for (size_t b = 0, be = bis.size(); b < be; ++b)
			{
				BlockInfo & block = *bis[b];
				block.m_tag = block.m_msg;
				//qDebug("consumed node: 0x%016x, tis_sz=%u bis_sz=%u b=%u block_msg=%s", node, tis.size(), bis.size(), b, block.m_msg.c_str());
				size_t const l = block.m_tag.find('[');
				size_t const r = block.m_tag.find(']');
				if (l != std::string::npos && r != std::string::npos)
					block.m_tag.erase(l, r - l + 1);

				colormap_t::iterator it = m_tagcolors.find(block.m_tag);
				if (it == m_tagcolors.end())
				{
					if (m_unique_colors.size() > 0)
					{
						block.m_color = m_unique_colors.back();
						m_unique_colors.pop_back();
					}
					m_tagcolors[block.m_tag] = block.m_color;
				}

				if (block.m_layer >= m_max_layers[t])
					m_max_layers[t] = block.m_layer;
			}

			int max_y = 0;
			int max_x = 0;

			int const h = g_heightValue;
			int const space = g_spaceValue;
			unsigned offs = 1;

			for (size_t b = 0, be = bis.size(); b < be; ++b)
			{
				BlockInfo & block = *bis[b];

				registerTag(block);

				int w = block.m_dt;
				qreal x = block.m_time_bgn;
				qreal y = (offs) * (h + space)  + block.m_layer * (h + space);
				block.m_x = x / g_scaleValue;
				block.m_y = y;
				qDebug("f=%2u t=%2u b=%2u  %s   (%3.2f, %3.2f) (x=%6.1f y=%6.1f w=%4i h=%4i dt=%3.3f)\n", block.m_frame, t, b, block.m_msg.c_str(), block.m_x, block.m_y, x, y, w, h, block.m_dt);
				//fflush(stdout);

				if (y > max_y)
					max_y = y;

				if (x > max_x)
					max_x = x;

				QGraphicsItem * item = new Bar(block, block.m_color, 0, 0, w, h, t, offs);
				item->setPos(QPointF(block.m_x, y));
				m_scene->addItem(item);
				item->setToolTip(QString("frame=%1 thread=%2 %3 [%4 ms]").arg(block.m_frame).arg(t).arg(block.m_msg.c_str()).arg(block.m_dt / 1000.0f));

				QGraphicsItem * titem = new BarText(block, block.m_color, 0, 0, w, h, t, offs);
				titem->setPos(QPointF(block.m_x, y));
				m_scene->addItem(titem);
			}

			offs += m_max_layers[t];
		}

		int const h = g_heightValue;
		int const space = g_spaceValue;
		for (size_t t = 0, te = tis.size(); t < te; ++t)
		{
			blockinfos_t const & bis = tis[t];

			for (size_t b = 0, be = bis.size(); b < be; ++b)
			{
				BlockInfo const & block = *bis[b];
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
	m_view->forceUpdate();
}

			/*	offs += max_layers[t] + 1;

				QPen p1;
				p1.setColor(Qt::gray);
				p1.setWidth(0);
				int y = (offs) * (h + space);
				QGraphicsLineItem * ln = new QGraphicsLineItem(0, y, max_x, y);
				ln->setPen(p1);
				m_scene->addItem(ln);

				offs += 1;
			}*/

/*
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
*/


/*	ProfileInfo & pi = m_profileInfo;

	//printf("p=%u\n", p); fflush(stdout);


	*/


		/*for (size_t f = 0, fe = pi.m_frames.size(); f < fe; ++f)
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
		}*/
/*
void ProfilerWindow::populateScene()
{
	qDebug("%s", __FUNCTION__);
	typedef std::map<std::string, QColor> colormap_t;
	colormap_t colors;


	ProfileInfo & pi = m_profileInfo;

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
		return;

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
				//printf("f=%2u t=%2u b=%2u    (%3.2f, %3.2f) (x=%6.1f y=%6.1f w=%4i h=%4i dt=%3.3f)\n", f, t, b, block.m_x, block.m_y, x, y, w, h, block.m_dt); fflush(stdout);

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
}*/

} // namespace profiler

