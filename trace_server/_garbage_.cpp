/*void LogWidget::findTextInAllColumns (QString const & text, int from_row, int to_row, bool only_first)
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
}*/

/*void LogWidget::findTextInColumn (QString const & text, int col, int from_row, int to_row)
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
}*/


