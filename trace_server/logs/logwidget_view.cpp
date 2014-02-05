#include "logwidget.h"
#include <QStatusBar>
//#include "logs/logtablemodel.h"
#include "filterproxymodel.h"
#include "utils.h"
#include "colorizermgr.h"

namespace logs {

    bool LogWidget::appendToColorizers (DecodedCommand const & cmd)
    {
        return colorizerMgr()->action(cmd);
    }


	void LogWidget::addColorTagRow (int src_row)
	{
		bool const checked = m_config_ui.hidePrevButton->isChecked();

		/*QModelIndex current = currentIndex();
		if (isModelProxy())
			current = m_proxy_model->mapToSource(current);
		if (!current.isValid())
			return;*/

		QString const & strrow = QString::number(src_row);

		colorizerMgr()->mkFilter(e_Colorizer_Row);

		if (checked)
			colorizerMgr()->getColorizerRow()->add(strrow, Qt::black, QColor(202, 225, 255));
		else
			colorizerMgr()->getColorizerRow()->remove(strrow);

		onInvalidateFilter(); //@TODO: should be done by filter?
	}

	bool LogWidget::findColorTagRow (int row) const
	{
		return false;
	}

	void LogWidget::removeColorTagRow (int row)
	{
	}


	/////////////////////////////////////////////
	void LogWidget::scrollToCurrentTag ()
	{
		/*if (m_config.m_auto_scroll)
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
		}*/
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
		/*if (m_color_tag_rows.size() > 0)
			scrollToCurrentTag();
		else*/
			scrollToCurrentSelection();
	}

	void LogWidget::nextToView ()
	{
		/*if (m_color_tag_rows.size() > 0)
		{
			++m_current_tag;
			scrollToCurrentTag();
		}
		else*/
		{
			++m_current_selection;
			scrollToCurrentSelection();
		}
	}

	/*


			{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetColorRegex()->model());
				QStandardItem * root = model->invisibleRootItem();
				for (int i = 0; i < sessionState().m_colorized_texts.size(); ++i)
				{
					ColorizedText & ct = sessionState().m_colorized_texts[i];
					ct.m_regex = QRegExp(ct.m_regex_str);

					QStandardItem * child = findChildByText(root, ct.m_regex_str);
					if (child == 0)
					{
						QList<QStandardItem *> row_items = addRow(ct.m_regex_str, ct.m_is_enabled);
						root->appendRow(row_items);
					}
				}
				recompileColorRegexps();
			}

			{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetRegex()->model());
				QStandardItem * root = model->invisibleRootItem();
				for (int i = 0; i < sessionState().m_filtered_regexps.size(); ++i)
				{
					FilteredRegex & flt = sessionState().m_filtered_regexps[i];
					flt.m_regex = QRegExp(flt.m_regex_str);

					QStandardItem * child = findChildByText(root, flt.m_regex_str);
					if (child == 0)
					{
						Qt::CheckState const state = flt.m_is_enabled ? Qt::Checked : Qt::Unchecked;
						QList<QStandardItem *> row_items = addTriRow(flt.m_regex_str, state, static_cast<bool>(flt.m_state));
						root->appendRow(row_items);
						child = findChildByText(root, flt.m_regex_str);
						child->setCheckState(state);
					}
				}
				recompileRegexps();
			}
			{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetString()->model());
				QStandardItem * root = model->invisibleRootItem();
				for (int i = 0; i < sessionState().m_filtered_strings.size(); ++i)
				{
					FilteredString & flt = sessionState().m_filtered_strings[i];
					appendToStringWidgets(flt);
				}
			}

		}

	*/

} // namespace logs



