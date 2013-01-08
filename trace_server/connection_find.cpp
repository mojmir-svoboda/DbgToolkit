#include "connection.h"
#include <QStatusBar>
#include "modelview.h"
#include "utils.h"

void Connection::findTextInAllColumns (QString const & text, int from_row, int to_row, bool only_first)
{
	ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
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

void Connection::findTextInColumn (QString const & text, int col, int from_row, int to_row)
{
	ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	for (int i = from_row, ie = to_row; i < ie; ++i)
	{
		QModelIndex const idx = model->index(i, col, QModelIndex());
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

			return;
		}
	}
	{
		qDebug("end of search");
		// flash icon
		m_main_window->statusBar()->showMessage(tr("End of document!"));
		m_last_search_row = 0;
	}
}

void Connection::selectionFromTo (int & from, int & to) const
{
	from = 0;
	ModelView const * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
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
	ModelView const * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
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
		ModelView const * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
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

void Connection::findNext ()
{
	int from, to;
	selectionFromTo(from, to);
	findTextInColumn(m_last_search, m_last_search_col, m_last_search_row + 1, to);
}

void Connection::findPrev ()
{
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

	ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());

	QModelIndex const model_idx = model->index(row_index.row(), idx, QModelIndex());
	if (model_idx.isValid())
	{
		QVariant value = model->data(model_idx);
		return value;
	}
	return QVariant();
}

