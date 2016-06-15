#pragma once

inline void findInWholeTable (QTableView * tableview, FindConfig const & fc, QModelIndexList & result)
{
	for (int i = 0, ie = tableview->model()->rowCount(); i < ie; ++i)
	{
		for (int j = 0, je = tableview->model()->columnCount(); j < je; ++j)
		{
			if (!fc.m_where.m_states[j])
				continue;

			QModelIndex const idx = tableview->model()->index(i, j, QModelIndex());
			QString s = tableview->model()->data(idx).toString();
			if (matchToFindConfig(s, fc))
				result.push_back(idx);
		}
	}
}

inline void currSelection (QTableView * tableview, QModelIndexList & sel)
{
	sel.clear();
	QItemSelectionModel const * selection = tableview->selectionModel();
	QItemSelection const sele = selection->selection();
	QModelIndexList const & sel2 = sele.indexes();
	//QModelIndexList sel2  = selection->selectedIndexes();
	if (sel2.size() < 1)
		return;
	/*
	QAbstractItemModel * m = model();
	QItemSelectionModel * selection = selectionModel();
	if (!selection)
		return QString();
	QModelIndexList indexes = selection->selectedIndexes();

	if (indexes.size() < 1)
		return QString();

	std::sort(indexes.begin(), indexes.end());*/

	// @FIXME: when proxy is on, there are invalid indexes in the current selection
	for(int i = 0, ie = sel2.size(); i < ie; ++i)
	{
		if (sel2.at(i).isValid())
			sel.push_back(sel2.at(i));
	}

  std::sort(sel.begin(), sel.end());
}
inline void uniqueRowsFromCurrSelection (QTableView * tableview, std::vector<QModelIndex> & sel)
{
	sel.clear();
	QItemSelectionModel const * selection = tableview->selectionModel();
	QItemSelection const sele = selection->selection();
	QModelIndexList const & sel2 = sele.indexes();
	if (sel2.size() < 1)
		return;

	// @FIXME: when proxy is on, there are invalid indexes in the current selection
	for (int i = 0, ie = sel2.size(); i < ie; ++i)
	{
		if (sel2.at(i).isValid())
		{
			QModelIndex const & sel2_idx = sel2.at(i);
			QModelIndex const sel2_idx0 = sel2_idx.model()->index(sel2_idx.row(), 0, QModelIndex());
			sel.push_back(sel2_idx0);
		}
	}

	std::sort(sel.begin(), sel.end());
	sel.erase(std::unique(sel.begin(), sel.end()), sel.end());
}

inline void findAndSelectNext (TableView * tableview, FindConfig const & fc)
{
	std::vector<QModelIndex> l;
	uniqueRowsFromCurrSelection(tableview, l);

	QModelIndex curr_idx;
	if (l.size())
		curr_idx = l[l.size() - 1];
	else
		curr_idx = tableview->model()->index(0, 0, QModelIndex());

	QModelIndex const idx = tableview->model()->index(curr_idx.row() + 1, curr_idx.column(), QModelIndex());
	if (!idx.isValid())
	{
		tableview->showWarningSign();
		return;
	}

	QModelIndexList next;
	for (int i = idx.row(), ie = tableview->model()->rowCount(); i < ie; ++i)
	{
		for (int j = 0, je = tableview->model()->columnCount(); j < je; ++j)
		{
			if (!fc.m_where.m_states[j])
				continue;

			QModelIndex const idx = tableview->model()->index(i, j, QModelIndex());
			QString s = tableview->model()->data(idx).toString();
			if (matchToFindConfig(s, fc))
			{
				next.push_back(idx);
				break;
			}
		}

		if (next.size() > 0)
			break;
	}

	if (next.size() == 0)
	{
		tableview->showWarningSign();
	}
	else
	{
		QItemSelectionModel * selection_model = tableview->selectionModel();
		QItemSelection selection;
		foreach(QModelIndex index, next)
		{
			QModelIndex left = tableview->model()->index(index.row(), 0);
			QModelIndex right = tableview->model()->index(index.row(), tableview->model()->columnCount() - 1);

			QItemSelection sel(left, right);
			selection.merge(sel, QItemSelectionModel::Select);
		}
		selection_model->clearSelection();
		selection_model->select(selection, QItemSelectionModel::Select);
		tableview->scrollTo(next.at(0), QTableView::PositionAtCenter);
	}
}

inline void findAndSelectPrev (TableView * tableview, FindConfig const & fc)
{
	std::vector<QModelIndex> l;
	uniqueRowsFromCurrSelection(tableview, l);

	if (l.size())
	{
		QModelIndex const & curr_idx = l.at(0);
		QModelIndex const idx = tableview->model()->index(curr_idx.row() - 1, curr_idx.column(), QModelIndex());
		if (!idx.isValid())
		{
			tableview->showWarningSign();
			return;
		}

		QModelIndexList next;
		/// ????
		for (int i = curr_idx.row(); i --> 0; )
		{
			for (int j = tableview->model()->columnCount(); j --> 0; )
			{
				if (!fc.m_where.m_states[j])
					continue;

				QModelIndex const idx = tableview->model()->index(i, j, QModelIndex());
				QString s = tableview->model()->data(idx).toString();
				if (matchToFindConfig(s, fc))
				{
					next.push_back(idx);
					break;
				}
			}
			if (next.size() > 0)
				break;
		}

		if (next.size() == 0)
		{
			tableview->showWarningSign();
		}
		else
		{
			QItemSelectionModel * selection_model = tableview->selectionModel();
			QItemSelection selection;
			foreach(QModelIndex index, next)
			{
				QModelIndex left = tableview->model()->index(index.row(), 0);
				QModelIndex right = tableview->model()->index(index.row(), tableview->model()->columnCount() - 1);

				QItemSelection sel(left, right);
				selection.merge(sel, QItemSelectionModel::Select);
			}
			selection_model->clearSelection();
			selection_model->select(selection, QItemSelectionModel::Select);
			tableview->scrollTo(next.at(0), QTableView::PositionAtCenter);
		}
	}
}



