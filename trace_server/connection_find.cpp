#include "connection.h"
#include <QStatusBar>
#include "logtablemodel.h"
#include "utils.h"

void Connection::findTextInAllColumns (QString const & text, int from_row, int to_row, bool only_first)
{
	LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	for (int i = from_row, ie = to_row; i < ie; ++i)
	{
		for (int j = 0, je = model->columnCount(); j < je; ++j)
		{
			if (isModelProxy()) // @TODO: dedup!
			{
				QModelIndex const idx = model->index(i, j, QModelIndex());
				QModelIndex const curr = m_table_view_proxy->mapFromSource(idx);

				if (idx.isValid() && model->data(idx).toString().contains(text, Qt::CaseInsensitive))
				{
					m_table_view_widget->selectionModel()->setCurrentIndex(curr, QItemSelectionModel::Select);
					m_last_search_row = idx.row();
					m_last_search_col = idx.column();
					if (only_first)
						return;
				}
			}
			else
			{
				QModelIndex const idx = model->index(i, j, QModelIndex());
				if (idx.isValid() && model->data(idx).toString().contains(text, Qt::CaseInsensitive))
				{
					m_table_view_widget->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Select);
					m_last_search_row = idx.row();
					m_last_search_col = idx.column();
					if (only_first)
						return;
				}
			}
		}
	}
}

bool Connection::matchTextInCell (QString const & text, int row, int col)
{
	LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	QModelIndex const idx = model->index(row, col, QModelIndex());
	if (idx.isValid() && model->data(idx).toString().contains(text, Qt::CaseInsensitive))
	{
		if (m_table_view_proxy)
		{
			QModelIndex const curr = m_table_view_proxy->mapFromSource(idx);
			m_table_view_widget->selectionModel()->setCurrentIndex(curr, QItemSelectionModel::Select);
			m_table_view_widget->scrollTo(m_table_view_proxy->mapFromSource(idx), QTableView::PositionAtCenter);
		}
		else
		{
			m_table_view_widget->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Select);
			m_table_view_widget->scrollTo(idx, QTableView::PositionAtCenter);
		}
		m_last_search_row = idx.row();
		return true;
	}
	return false;
}

void Connection::endOfSearch ()
{
	qDebug("end of search");
	// flash icon
	m_main_window->statusBar()->showMessage(tr("End of document!"));
	m_last_search_row = 0;
}

void Connection::findTextInColumn (QString const & text, int col, int from_row, int to_row)
{
	for (int i = from_row, ie = to_row; i < ie; ++i)
		if (matchTextInCell(text, i, col))
			return;
	endOfSearch();
}
void Connection::findTextInColumnRev (QString const & text, int col, int from_row, int to_row)
{
	bool found = false;
	for (int i = from_row, ie = to_row; i --> ie; )
		if (matchTextInCell(text, i, col))
			return;

	endOfSearch();
}


void Connection::selectionFromTo (int & from, int & to) const
{
	from = 0;
	LogTableModel const * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	to = model->rowCount();
	QItemSelectionModel const * selection = m_table_view_widget->selectionModel();
	QModelIndexList indexes = selection->selectedIndexes();
	if (indexes.size() < 1)
		return;

	std::sort(indexes.begin(), indexes.end());
	from = indexes.first().row();
}

void Connection::findAllTexts (QString const & text)
{
	m_last_search = text;
	int from = 0;
	LogTableModel const * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	int to = model->rowCount();
	findTextInAllColumns(text, from, to, false);
}

void Connection::findText (QString const & text, tlv::tag_t tag)
{
	if (m_last_search != text)
	{
		m_last_search_row = 0;	// this is a new search
		m_last_search = text;
		int const col_idx = sessionState().findColumn4Tag(tag);
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
		LogTableModel const * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
		int const to = model->rowCount();
		findTextInColumn(m_last_search, m_last_search_col, m_last_search_row + 1, to);
	}
}

void Connection::findText (QString const & text)
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

void Connection::findNext (QString const & text)
{
	int from, to;
	selectionFromTo(from, to);
	if (text != m_last_search)
	{
		m_last_search = text;
	}

	if (!m_last_clicked.isValid())
	{
		int const col_idx = sessionState().findColumn4Tag(tlv::tag_msg);
		m_last_search_col = col_idx < 0 ? 0 : col_idx;
	}

	if (m_last_search.isEmpty())
	{
		m_last_search_row = 0;
		return;
	}
	findTextInColumn(m_last_search, m_last_search_col, m_last_search_row + 1, to);
}

void Connection::findPrev (QString const & text)
{
	int from, to;
	selectionFromTo(from, to);
	if (!m_last_clicked.isValid())
	{
		int const col_idx = sessionState().findColumn4Tag(tlv::tag_msg);
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

QString Connection::findString4Tag (tlv::tag_t tag, QModelIndex const & row_index) const
{
	return findVariant4Tag(tag, row_index).toString();
}

QVariant Connection::findVariant4Tag (tlv::tag_t tag, QModelIndex const & row_index) const
{
	int const idx = sessionState().m_tags2columns[tag];
	if (idx == -1)
		return QVariant();

	LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());

	QModelIndex const model_idx = model->index(row_index.row(), idx, QModelIndex());
	if (model_idx.isValid())
	{
		QVariant value = model->data(model_idx);
		return value;
	}
	return QVariant();
}


void Connection::scrollToCurrentTag ()
{
	if (m_main_window->autoScrollEnabled())
		return;

	if (sessionState().m_color_tag_rows.size() == 0)
		return;

	if (sessionState().m_current_tag == -1)
		sessionState().m_current_tag = 0;

	if (sessionState().m_current_tag >= sessionState().m_color_tag_rows.size())
		sessionState().m_current_tag = 0;

	if (sessionState().m_current_tag < sessionState().m_color_tag_rows.size())
	{
		int const tag_row = sessionState().m_color_tag_rows[sessionState().m_current_tag];
		QModelIndex const tag_idx = m_table_view_widget->model()->index(tag_row, 0);

		//qDebug("scrollToCurrentTag: current=%2i src row=%2i ", sessionState().m_current_tag, tag_row);

		if (isModelProxy())
			m_table_view_widget->scrollTo(m_table_view_proxy->mapFromSource(tag_idx), QAbstractItemView::PositionAtCenter);
		else
			m_table_view_widget->scrollTo(tag_idx, QAbstractItemView::PositionAtCenter);
	}
}

void Connection::scrollToCurrentSelection ()
{
	if (m_main_window->autoScrollEnabled())
		return;

	QItemSelectionModel const * selection = m_table_view_widget->selectionModel();
	QModelIndexList indexes = selection->selectedIndexes();

	if (indexes.size() == 0)
		return;

	if (sessionState().m_current_selection == -1)
		sessionState().m_current_selection = 0;

	if (sessionState().m_current_selection >= indexes.size())
		sessionState().m_current_selection = 0;

	QModelIndex const idx = indexes.at(sessionState().m_current_selection);
	qDebug("scrollToSelection[%i] row=%i", sessionState().m_current_selection, idx.row());
	if (isModelProxy())
	{
		QModelIndex const own_idx = m_table_view_proxy->index(idx.row(), idx.column());
		m_table_view_widget->scrollTo(own_idx, QAbstractItemView::PositionAtCenter);
	}
	else
	{
		QModelIndex const own_idx = m_table_view_widget->model()->index(idx.row(), idx.column());
		m_table_view_widget->scrollTo(own_idx, QAbstractItemView::PositionAtCenter);
	}
}

void Connection::scrollToCurrentTagOrSelection ()
{
	if (sessionState().m_color_tag_rows.size() > 0)
		scrollToCurrentTag();
	else
		scrollToCurrentSelection();
}

void Connection::nextToView ()
{
	if (sessionState().m_color_tag_rows.size() > 0)
	{
		++sessionState().m_current_tag;
		scrollToCurrentTag();
	}
	else
	{
		++sessionState().m_current_selection;
		scrollToCurrentSelection();
	}
}

void Connection::onFindFileLine (QModelIndex const &)
{
	//@FIXME: unused args
	qDebug("find file:line for idx=(%i,col)", m_last_clicked.row());

	bool const scroll_to_item = true;
	bool const expand = true;
	findTableIndexInFilters(m_last_clicked, scroll_to_item, expand);
}

