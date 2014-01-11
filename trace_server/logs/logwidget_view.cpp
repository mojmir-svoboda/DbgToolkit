#include "logwidget.h"
#include <QStatusBar>
//#include "logs/logtablemodel.h"
#include <logs/filterproxymodel.h>
//#include <logs/findproxymodel.h>
#include "utils.h"
//#include "connection.h"
//#include "warnimage.h"

namespace logs {

	void LogWidget::setupColorRegex ()
	{
		m_config_ui.ui()->viewColorRegex->setEditTriggers(QAbstractItemView::NoEditTriggers);
		connect(m_config_ui.ui()->viewColorRegex, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtColorRegexList(QModelIndex)));
		connect(m_config_ui.ui()->viewColorRegex, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedAtColorRegexList(QModelIndex)));
		connect(m_config_ui.ui()->comboBoxColorRegex, SIGNAL(activated(int)), this, SLOT(onColorRegexActivate(int)));
		connect(m_config_ui.ui()->buttonAddColorRegex, SIGNAL(clicked()), this, SLOT(onColorRegexAdd()));
		connect(m_config_ui.ui()->buttonRmColorRegex, SIGNAL(clicked()), this, SLOT(onColorRegexRm()));

		if (!m_color_regex_model)
			m_color_regex_model = new QStandardItemModel;
		m_config_ui.ui()->viewColorRegex->setModel(m_color_regex_model);
		m_color_regex_model->setColumnCount(4);
		m_config_ui.ui()->viewColorRegex->setColumnWidth(0, 192);
	}

	void LogWidget::appendToColorRegex (QString const & val)
	{
		m_filter_state.appendToColorRegexFilters(val);
	}

	void LogWidget::removeFromColorRegex (QString const & val)
	{
		m_filter_state.removeFromColorRegexFilters(val);
	}

	void LogWidget::updateColorRegex ()
	{
		for (size_t r = 0, re = m_src_model->dcmds().size(); r < re; ++r)
		{
			DecodedCommand const & dcmd = m_src_model->dcmds()[r];

			bool row_match = false;
			for (size_t i = 0, ie = dcmd.m_tvs.size(); i < ie; ++i)
			{
				QString const & val = dcmd.m_tvs[i].m_val;

				QColor color, bgcolor;
				bool const is_match = m_filter_state.isMatchedColorizedText(val, color, bgcolor);
				QModelIndex const idx = m_src_model->index(r, i, QModelIndex());

				m_src_model->setData(idx, QColor(Qt::white), Qt::BackgroundRole);
				m_src_model->setData(idx, QColor(Qt::black), Qt::ForegroundRole);
				if (is_match)
				{
					m_src_model->setData(idx, bgcolor, Qt::BackgroundRole);
					m_src_model->setData(idx, color, Qt::ForegroundRole);
				}
			}
			//@TODO: if column != level
			//@TODO: if column != tid
		}
	}

	/*void LogWidget::loadToColorRegexps (QString const & filter_item, QString const & color, bool enabled)
	{
		//m_filter_state.appendToColorRegexFilters(filter_item);
		//m_filter_state.setRegexColor(filter_item, QColor(color));
		//m_filter_state.setRegexChecked(filter_item, enabled);
	}*/

	void LogWidget::onFgColorRegexChanged () { onColorRegexChanged(Qt::ForegroundRole); }
	void LogWidget::onBgColorRegexChanged () { onColorRegexChanged(Qt::BackgroundRole); }
	void LogWidget::onColorRegexChanged (int role)
	{
		for (int i = 0, ie = m_filter_state.m_colorized_texts.size(); i < ie; ++i)
		{
			ColorizedText & ct = m_filter_state.m_colorized_texts[i];
			QStandardItem * const root = m_color_regex_model->invisibleRootItem();
			QString const qregex = ct.m_regex_str;
			QStandardItem * const child = findChildByText(root, qregex);

			if (!child)
				continue;

			QModelIndex const fgidx = m_color_regex_model->index(child->row(), 1);
			if (fgidx.isValid())
			{
				if (QtColorPicker * w = static_cast<QtColorPicker *>(m_config_ui.ui()->viewColorRegex->indexWidget(fgidx)))
					ct.m_qcolor = w->currentColor();
			}
			QModelIndex const bgidx = m_color_regex_model->index(child->row(), 2);
			if (bgidx.isValid())
			{
				if (QtColorPicker * w = static_cast<QtColorPicker *>(m_config_ui.ui()->viewColorRegex->indexWidget(bgidx)))
					ct.m_bgcolor = w->currentColor();
			}
		}
		updateColorRegex();
	}

	QtColorPicker * mkColorPicker (QWidget * parent, QString const & txt, QColor const & c)
	{
		QtColorPicker * w = new QtColorPicker(parent, txt);
		w->setStandardColors();
		w->setCurrentColor(c);
		return w;
	}

	void LogWidget::recompileColorRegexps ()
	{
		for (int i = 0, ie = m_filter_state.m_colorized_texts.size(); i < ie; ++i)
		{
			ColorizedText & ct = m_filter_state.m_colorized_texts[i];
			QStandardItem * root = m_color_regex_model->invisibleRootItem();
			QString const qregex = ct.m_regex_str;
			QStandardItem * child = findChildByText(root, qregex);
			QModelIndex const idx = m_color_regex_model->indexFromItem(child);
			ct.m_is_enabled = false;
			if (!child)
				continue;

			QRegExp regex(qregex);
			QString reason;
			if (regex.isValid())
			{
				ct.m_regex = regex;

				bool const checked = (child->checkState() == Qt::Checked);
				if (child && checked)
				{
					child->setData(QBrush(Qt::green), Qt::BackgroundRole);
					reason = "ok";
					ct.m_is_enabled = true;
				}
				else if (child && !checked)
				{
					child->setData(QBrush(Qt::yellow), Qt::BackgroundRole);
					reason = "not checked";
				}
			}
			else
			{
				if (child)
				{
					child->setData(QBrush(Qt::red), Qt::BackgroundRole);
					reason = regex.errorString();
				}
			}

			child->setToolTip(reason);
			QStandardItem * item = m_color_regex_model->item(child->row(), 3);
			item->setText(reason);
		}

		updateColorRegex();
	}


	void LogWidget::onDoubleClickedAtColorRegexList (QModelIndex idx)
	{
	}

	void LogWidget::onClickedAtColorRegexList (QModelIndex idx)
	{
		if (!idx.isValid()) return;
		QStandardItemModel * model = static_cast<QStandardItemModel *>(m_config_ui.ui()->viewColorRegex->model());
		QStandardItem * item = model->itemFromIndex(idx);
		Q_ASSERT(item);

		QString const & val = model->data(idx, Qt::DisplayRole).toString();
		bool const orig_checked = (item->checkState() == Qt::Checked);
		Qt::CheckState const checked = orig_checked ? Qt::Unchecked : Qt::Checked;
		qDebug("color regex click! (checked=%u) %s ", checked, val.toStdString().c_str());
		item->setCheckState(checked);

		// @TODO: if state really changed
		m_filter_state.setColorRegexChecked(val, checked);
		recompileColorRegexps();
		updateColorRegex();
	}

	void LogWidget::onColorRegexActivate (int idx)
	{
	}

	void LogWidget::onClearCurrentColorizedRegex ()
	{
	}

	void LogWidget::onColorRegexAdd ()
	{
		QString qItem = m_config_ui.ui()->comboBoxColorRegex->currentText();
		if (!qItem.length())
			return;
		QStandardItemModel * model = static_cast<QStandardItemModel *>(m_config_ui.ui()->viewColorRegex->model());
		QStandardItem * root = model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, qItem);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addRow(qItem, false);
			root->appendRow(row_items);
			child = findChildByText(root, qItem);

			QStandardItem * fgitem = new QStandardItem("fg");
			QStandardItem * bgitem = new QStandardItem("bg");
			QStandardItem * stitem = new QStandardItem("status");
			stitem->setCheckable(false);
			model->setItem(child->row(), 1, fgitem);
			model->setItem(child->row(), 2, bgitem);
			model->setItem(child->row(), 3, stitem);
			appendToColorRegex(qItem);

			for (int i = 0, ie = m_filter_state.m_colorized_texts.size(); i < ie; ++i)
			{
				ColorizedText & ct = m_filter_state.m_colorized_texts[i];
				if (ct.m_regex_str == qItem)
				{
					{
						QtColorPicker * w = mkColorPicker(m_config_ui.ui()->viewColorRegex, "fg", ct.m_qcolor);
						connect(w, SIGNAL(colorChanged(const QColor &)), this, SLOT(onFgColorRegexChanged()));
						QModelIndex const idx = model->indexFromItem(fgitem);
						m_config_ui.ui()->viewColorRegex->setIndexWidget(idx, w);
					}
					{
						QtColorPicker * w = mkColorPicker(m_config_ui.ui()->viewColorRegex, "bg", ct.m_bgcolor);
						connect(w, SIGNAL(colorChanged(const QColor &)), this, SLOT(onBgColorRegexChanged()));
						QModelIndex const idx = model->indexFromItem(bgitem);
						m_config_ui.ui()->viewColorRegex->setIndexWidget(idx, w);
					}
					break;
				}
			}
		}
		recompileColorRegexps();
	}

	void LogWidget::onColorRegexRm ()
	{
		QStandardItemModel * model = static_cast<QStandardItemModel *>(m_config_ui.ui()->viewColorRegex->model());
		QModelIndex const idx = m_config_ui.ui()->viewColorRegex->currentIndex();
		QStandardItem * item = model->itemFromIndex(idx);
		if (!item)
			return;
		QString const & val = model->data(idx, Qt::DisplayRole).toString();
		model->removeRow(idx.row());

		removeFromColorRegex(val);
		recompileColorRegexps();
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



