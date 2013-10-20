#include "logwidget.h"
#include <QStatusBar>
#include "logs/logtablemodel.h"
#include <logs/filterproxymodel.h>
#include <logs/findproxymodel.h>
#include "utils.h"
#include "connection.h"

namespace logs {

void LogWidget::findInWholeTable (FindConfig const & fc, QModelIndexList & result)
{
	for (int i = 0, ie = model()->rowCount(); i < ie; ++i)
	{
		for (int j = 0, je = model()->columnCount(); j < je; ++j)
		{
			QModelIndex const idx = model()->index(i, j, QModelIndex());
			QModelIndex src_idx = idx; 
			if (isModelProxy())
			{
				src_idx = m_proxy_model->mapFromSource(idx);
			}

			if (fc.m_regexp)
			{
			}
			else
			{
				Qt::CaseSensitivity const cs = fc.m_case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
				if (idx.isValid() && model()->data(idx).toString().contains(fc.m_str, cs))
				{
					result.push_back(src_idx);
				}
			}
		}
	}
}

void LogWidget::registerLinkedLog (LogWidget * l)
{
	m_linked_widgets.push_back(l);
}

void LogWidget::unregisterLinkedLog (LogWidget * l)
{
	m_linked_widgets.erase(std::remove(m_linked_widgets.begin(), m_linked_widgets.end(), l), m_linked_widgets.end());
}

void LogWidget::linkToSource (LogWidget * src)
{
	m_linked_parent = src;
	src->registerLinkedLog(this);

	/*setStyleSheet("QHeaderView::section, QHeaderView::section * {\
		background-color: yellow\
		border: 1px solid #6c6c6c;\
	}");*/
}

LogWidget * LogWidget::mkFindAllRefsLogWidget (FindConfig const & fc)
{
	QString tag;
	if (fc.m_to_widget.isEmpty())
		tag = "find_all_refs";
	else
	{
		// @TODO: validate widget form: appname/foo
		tag = fc.m_to_widget;
	}

	LogConfig cfg;
	cfg.m_tag = tag;
	bool const loaded = m_connection->dataWidgetConfigPreload<e_data_log>(tag, cfg);
	if (!loaded)
	{
		cfg = m_config;	// inherit from parent widget if not loaded
	}
	cfg.m_tag = tag;
	cfg.m_show = true;
	cfg.m_central_widget = false;
	cfg.m_filter_proxy = false;
	cfg.m_find_proxy = true;
	datalogs_t::iterator it = m_connection->dataWidgetFactoryFrom<e_data_log>(tag, cfg);

	DataLog * dp = *it;
	LogWidget & child = dp->widget();
	child.linkToSource(this);
	/* QString const logpath = preset_dir + "/" + g_presetLogTag;
		m_config.clear();
		bool const loaded = logs::loadConfig(m_config, logpath + "/" + m_config.m_tag);
		if (!loaded)
			m_connection->defaultConfigFor(m_config);
		filterMgr()->loadConfig(logpath);*/
	child.setupLogModel(m_src_model);
	child.setFindProxyModel(fc);
	dp->m_wd->setStyleSheet("\
			QHeaderView::section {\
			background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,\
											  stop:0 #616161, stop: 0.5 #505050,\
											  stop: 0.6 #434343, stop:1 #656565);\
			color: white;\
			padding-left: 4px;\
			border: 1px solid #6c6c6c;\
		}");

	//child.setSelectionModel(m_selection);
	//@FIXME: natvrdo m_proxy_model... spatne!
	child.m_kselection_model = new KLinkItemSelectionModel(m_proxy_model, child.selectionModel());
	setSelectionModel(child.m_kselection_model);
	
	return &child;
}

LogWidget * LogWidget::mkFindAllCloneLogWidget (FindConfig const & fc)
{
	QString tag;
	if (fc.m_to_widget.isEmpty())
	{
		if (fc.m_refs)
			tag = "find_all_refs";
		else if (fc.m_clone)
			tag = "find_all_clone";
		else
			tag = "noname";
	}
	else
	{
		// @TODO: validate widget form: appname/foo
		tag = fc.m_to_widget;
	}

	LogConfig cfg;
	cfg.m_tag = tag;
	bool const loaded = m_connection->dataWidgetConfigPreload<e_data_log>(tag, cfg);
	if (!loaded)
	{
		cfg = m_config;	// inherit from parent widget if not loaded
	}
	cfg.m_tag = tag;
	cfg.m_show = true;
	cfg.m_central_widget = false;
	cfg.m_filter_proxy = false;
	cfg.m_find_proxy = false;
	datalogs_t::iterator it = m_connection->dataWidgetFactoryFrom<e_data_log>(tag, cfg);

	DataLog * dp = *it;
	LogWidget & child = dp->widget();
	//child.linkToSource(this);
	/* QString const logpath = preset_dir + "/" + g_presetLogTag;
		m_config.clear();
		bool const loaded = logs::loadConfig(m_config, logpath + "/" + m_config.m_tag);
		if (!loaded)
			m_connection->defaultConfigFor(m_config);
		filterMgr()->loadConfig(logpath);*/
	LogTableModel * clone_model = m_src_model->cloneToNewModel();
	child.setupLogModel(clone_model);
	child.setSrcModel(fc);

	//child.setFindProxyModel(fc);

	//child.setSelectionModel(m_selection);
	//@FIXME: natvrdo m_proxy_model... spatne!
	//child.m_kselection_model = new KLinkItemSelectionModel(m_proxy_model, child.selectionModel());
	//setSelectionModel(child.m_kselection_model);
	
	return &child;
}

void LogWidget::setSrcModel (FindConfig const & fc)
{
	m_config.m_find_config = fc;
	setModel(m_src_model);
	m_src_model->resizeToCfg();
	m_find_proxy_selection = new QItemSelectionModel(m_find_proxy_model);
	setSelectionModel(m_src_selection);
	resizeSections();
	applyConfig();
}

void LogWidget::setFindProxyModel (FindConfig const & fc)
{
	m_config.m_find_config = fc;
	setModel(m_find_proxy_model);
	m_find_proxy_model->force_update();
	m_find_proxy_model->resizeToCfg();
	m_find_proxy_selection = new QItemSelectionModel(m_find_proxy_model);
	setSelectionModel(m_find_proxy_selection);
	resizeSections();
	applyConfig();
}

void LogWidget::handleFindAction (FindConfig const & fc)
{
	bool const select_only = !fc.m_refs && !fc.m_clone;

	if (select_only)
	{
		// select(results);
	}
	else
	{
		LogWidget * result_widget = 0;
		if (fc.m_refs)
		{
			result_widget = mkFindAllRefsLogWidget(fc);
			// setup selection
		}
		else // clone
		{
			result_widget = mkFindAllCloneLogWidget(fc);
		}
	}
}


void LogWidget::findTextInAllColumns (QString const & text, int from_row, int to_row, bool only_first)
{
	for (int i = from_row, ie = to_row; i < ie; ++i)
	{
		for (int j = 0, je = model()->columnCount(); j < je; ++j)
		{
			if (isModelProxy()) // @TODO: dedup!
			{
				QModelIndex const idx = model()->index(i, j, QModelIndex());
				QModelIndex const curr = m_proxy_model->mapFromSource(idx);

				if (idx.isValid() && model()->data(idx).toString().contains(text, Qt::CaseInsensitive))
				{
					selectionModel()->setCurrentIndex(curr, QItemSelectionModel::Select);
					m_last_search_row = idx.row();
					m_last_search_col = idx.column();
					//m_last_search_idx = idx;
					if (only_first)
						return;
				}
			}
			else
			{
				QModelIndex const idx = model()->index(i, j, QModelIndex());
				if (idx.isValid() && model()->data(idx).toString().contains(text, Qt::CaseInsensitive))
				{
					selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Select);
					m_last_search_row = idx.row();
					m_last_search_col = idx.column();
					if (only_first)
						return;
				}
			}
		}
	}
}

bool LogWidget::matchTextInCell (QString const & text, int row, int col)
{
	LogTableModel * model = m_src_model;
	QModelIndex const idx = model->index(row, col, QModelIndex());
	if (idx.isValid() && model->data(idx).toString().contains(text, Qt::CaseInsensitive))
	{
		qDebug("found string %s: src=%i,%i", text.toStdString(), row, col);
		if (isModelProxy()) // @TODO: dedup!
		{
			QModelIndex const curr = m_proxy_model->mapFromSource(idx);
			selectionModel()->setCurrentIndex(curr, QItemSelectionModel::Select);
			scrollTo(m_proxy_model->mapFromSource(idx), QTableView::PositionAtCenter);
		}
		else
		{
			selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Select);
			scrollTo(idx, QTableView::PositionAtCenter);
		}
		m_last_search_row = idx.row();
		return true;
	}
	return false;
}

void LogWidget::endOfSearch ()
{
	qDebug("end of search");
	// flash icon
	//m_connection->getMainWindow()->statusBar()->showMessage(tr("End of document!"));
	m_last_search_row = 0;
}

void LogWidget::findTextInColumn (QString const & text, int col, int from_row, int to_row)
{
	for (int i = from_row, ie = to_row; i < ie; ++i)
		if (matchTextInCell(text, i, col))
			return;
	endOfSearch();
}
void LogWidget::findTextInColumnRev (QString const & text, int col, int from_row, int to_row)
{
	bool found = false;
	for (int i = from_row, ie = to_row; i --> ie; )
		if (matchTextInCell(text, i, col))
			return;

	endOfSearch();
}


void LogWidget::selectionFromTo (int & from, int & to) const
{
	from = 0;
	LogTableModel const * model = m_src_model;
	to = model->rowCount();
	QItemSelectionModel const * selection = selectionModel();
	QModelIndexList indexes = selection->selectedIndexes();
	if (indexes.size() < 1)
		return;

	std::sort(indexes.begin(), indexes.end());
	from = indexes.first().row();
}

void LogWidget::findAllTexts (QString const & text)
{
	m_last_search = text;
	int from = 0;
	LogTableModel const * model = m_src_model;
	int to = model->rowCount();
	findTextInAllColumns(text, from, to, false);
}

void LogWidget::findText (QString const & text, tlv::tag_t tag)
{
	if (m_last_search != text)
	{
		m_last_search_row = 0;	// this is a new search
		m_last_search = text;
		int const col_idx = findColumn4Tag(tag);
		m_last_search_col = col_idx;

		if (m_last_search.isEmpty())
		{
			m_last_search_row = m_last_search_col = 0;
			return;
		}


		//@TODO: clear selection?
		int from, to;
		selectionFromTo(from, to);
		findTextInColumn(m_last_search, col_idx, from, to);
	}
	else
	{
		LogTableModel const * model = m_src_model;
		int const to = model->rowCount();
		findTextInColumn(m_last_search, m_last_search_col, m_last_search_row + 1, to);
	}
}

void LogWidget::findText (QString const & text)
{
	m_last_search = text;
	m_last_search_row = 0;
	m_last_search_col = -1;

	if (m_last_search.isEmpty())
	{
		m_last_search_row = m_last_search_col = 0;
		return;
	}

	int from, to;
	selectionFromTo(from, to);
	findTextInAllColumns(m_last_search, from, to, true);
}

void LogWidget::findNext (QString const & text)
{
	int from, to;
	selectionFromTo(from, to);
	if (text != m_last_search)
	{
		m_last_search = text;
	}

	if (!m_last_clicked.isValid())
	{
		int const col_idx = findColumn4Tag(tlv::tag_msg);
		m_last_search_col = col_idx < 0 ? 0 : col_idx;
	}

	if (m_last_search.isEmpty())
	{
		m_last_search_row = 0;
		return;
	}
	findTextInColumn(m_last_search, m_last_search_col, m_last_search_row + 1, to);
}

void LogWidget::findPrev (QString const & text)
{
	int from, to;
	selectionFromTo(from, to);
	if (!m_last_clicked.isValid())
	{
		int const col_idx = findColumn4Tag(tlv::tag_msg);
		m_last_search_col = col_idx < 0 ? 0 : col_idx;
	}

	if (text != m_last_search)
	{
		m_last_search = text;
	}

	if (m_last_search.isEmpty())
	{
		m_last_search_row = to;
		return;
	}
	int const last = m_last_search_row > 0 ? m_last_search_row - 1 : to;
	findTextInColumnRev(m_last_search, m_last_search_col, last, 0);
}

QString LogWidget::findString4Tag (tlv::tag_t tag, QModelIndex const & row_index) const
{
	return findVariant4Tag(tag, row_index).toString();
}

QVariant LogWidget::findVariant4Tag (tlv::tag_t tag, QModelIndex const & row_index) const
{
	int const idx = m_tags2columns[tag];
	if (idx == -1)
		return QVariant();

	LogTableModel * model = m_src_model;

	QModelIndex const model_idx = model->index(row_index.row(), idx, QModelIndex());
	if (model_idx.isValid())
	{
		QVariant value = model->data(model_idx);
		return value;
	}
	return QVariant();
}


void LogWidget::scrollToCurrentTag ()
{
	if (m_config.m_auto_scroll)
		return;

	if (m_color_tag_rows.size() == 0)
		return;

	if (m_current_tag == -1)
		m_current_tag = 0;

	if (m_current_tag >= m_color_tag_rows.size())
		m_current_tag = 0;

	if (m_current_tag < m_color_tag_rows.size())
	{
		int const tag_row = m_color_tag_rows[m_current_tag];
		QModelIndex const tag_idx = model()->index(tag_row, 0);

		//qDebug("scrollToCurrentTag: current=%2i src row=%2i ", sessionState().m_current_tag, tag_row);

		if (isModelProxy())
			scrollTo(m_proxy_model->mapFromSource(tag_idx), QAbstractItemView::PositionAtCenter);
		else
			scrollTo(tag_idx, QAbstractItemView::PositionAtCenter);
	}
}

void LogWidget::scrollToCurrentSelection ()
{
	if (m_config.m_auto_scroll)
		return;

	QItemSelectionModel const * selection = selectionModel();
	QModelIndexList indexes = selection->selectedIndexes();

	if (indexes.size() == 0)
		return;

	if (m_current_selection == -1)
		m_current_selection = 0;

	if (m_current_selection >= indexes.size())
		m_current_selection = 0;

	QModelIndex const idx = indexes.at(m_current_selection);
	qDebug("scrollToSelection[%i] row=%i", m_current_selection, idx.row());
	if (isModelProxy())
	{
		QModelIndex const own_idx = m_proxy_model->index(idx.row(), idx.column());
		scrollTo(own_idx, QAbstractItemView::PositionAtCenter);
	}
	else
	{
		QModelIndex const own_idx = model()->index(idx.row(), idx.column());
		scrollTo(own_idx, QAbstractItemView::PositionAtCenter);
	}
}

void LogWidget::scrollToCurrentTagOrSelection ()
{
	if (m_color_tag_rows.size() > 0)
		scrollToCurrentTag();
	else
		scrollToCurrentSelection();
}

void LogWidget::nextToView ()
{
	if (m_color_tag_rows.size() > 0)
	{
		++m_current_tag;
		scrollToCurrentTag();
	}
	else
	{
		++m_current_selection;
		scrollToCurrentSelection();
	}
}

void LogWidget::onFindFileLine (QModelIndex const &)
{
	//@FIXME: unused args
	qDebug("find file:line for idx=(%i,col)", m_last_clicked.row());

	bool const scroll_to_item = true;
	bool const expand = true;
	findTableIndexInFilters(m_last_clicked, scroll_to_item, expand);
}

void LogWidget::addColorTagRow (int row)
{
	for (int i = 0, ie = m_color_tag_rows.size(); i < ie; ++i)
		if (m_color_tag_rows.at(i) == row)
		{
			removeColorTagRow(row);
			return;
		}
	m_color_tag_rows.push_back(row);
}

bool LogWidget::findColorTagRow (int row) const
{
	for (int i = 0, ie = m_color_tag_rows.size(); i < ie; ++i)
		if (m_color_tag_rows.at(i) == row)
			return true;
	return false;
}

void LogWidget::removeColorTagRow (int row)
{
	m_color_tag_rows.erase(std::remove(m_color_tag_rows.begin(), m_color_tag_rows.end(), row), m_color_tag_rows.end());
}




}
