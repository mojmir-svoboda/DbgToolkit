#include "logwidget.h"
#include <QStatusBar>
//#include "logs/logtablemodel.h"
#include <logs/filterproxymodel.h>
//#include <logs/findproxymodel.h>
#include "utils.h"
//#include "connection.h"
//#include "warnimage.h"

namespace logs {

    void LogWidget::appendToColorRegex (QString const & val)
    {
        m_filter_state.appendToColorRegexFilters(val);
    }

    void LogWidget::removeFromColorRegex (QString const & val)
    {
        m_filter_state.removeFromColorRegexFilters(val);
    }

    /*void LogWidget::loadToColorRegexps (QString const & filter_item, QString const & color, bool enabled)
    {
        //m_filter_state.appendToColorRegexFilters(filter_item);
        //m_filter_state.setRegexColor(filter_item, QColor(color));
        //m_filter_state.setRegexChecked(filter_item, enabled);
    }*/

    void LogWidget::onColorRegexChanged ()
    {
    /*	for (int i = 0, ie = m_filter_state.m_colorized_texts.size(); i < ie; ++i)
        {
            ColorizedText & ct = m_filter_state.m_colorized_texts[i];
            QStandardItem * root = filterWidget()->m_color_regex_model->invisibleRootItem();
            QString const qregex = ct.m_regex_str;
            QStandardItem * child = findChildByText(root, qregex);
            QModelIndex const idx = filterWidget()->m_color_regex_model->indexFromItem(child);
            if (!child)
                continue;

            if (QtColorPicker * w = static_cast<QtColorPicker *>(filterWidget()->getWidgetColorRegex()->indexWidget(idx)))
            {
                ct.m_qcolor = w->currentColor();
            }
        }
        onInvalidateFilter();*/
    }

    void LogWidget::recompileColorRegexps ()
    {
        /*for (int i = 0, ie = m_filter_state.m_colorized_texts.size(); i < ie; ++i)
        {
            ColorizedText & ct = m_filter_state.m_colorized_texts[i];
            QStandardItem * root = filterWidget()->m_color_regex_model->invisibleRootItem();
            QString const qregex = ct.m_regex_str;
            QStandardItem * child = findChildByText(root, qregex);
            QModelIndex const idx = filterWidget()->m_color_regex_model->indexFromItem(child);
            ct.m_is_enabled = false;
            if (!child)
                continue;

            if (filterWidget()->getWidgetColorRegex()->indexWidget(idx) == 0)
            {
                QtColorPicker * w = new QtColorPicker(filterWidget()->getWidgetColorRegex(), qregex);
                w->setStandardColors();
                w->setCurrentColor(ct.m_qcolor);

                connect(w, SIGNAL(colorChanged(const QColor &)), this, SLOT(onColorRegexChanged()));
                filterWidget()->getWidgetColorRegex()->setIndexWidget(idx, w);
            }
            else
            {
                QtColorPicker * w = static_cast<QtColorPicker *>(filterWidget()->getWidgetColorRegex()->indexWidget(idx));
                w->setCurrentColor(ct.m_qcolor);
            }

            QRegExp regex(qregex);
            if (regex.isValid())
            {
                ct.m_regex = regex;

                bool const checked = (child->checkState() == Qt::Checked);
                if (child && checked)
                {
                    child->setData(QBrush(Qt::green), Qt::BackgroundRole);
                    child->setToolTip(tr("ok"));
                    ct.m_is_enabled = true;
                }
                else if (child && !checked)
                {
                    child->setData(QBrush(Qt::yellow), Qt::BackgroundRole);
                    child->setToolTip(tr("not checked"));
                }
            }
            else
            {
                if (child)
                {
                    child->setData(QBrush(Qt::red), Qt::BackgroundRole);
                    child->setToolTip(regex.errorString());
                }
            }
        }

        onInvalidateFilter();*/
    }


    void LogWidget::onDoubleClickedAtColorRegexList (QModelIndex idx)
    {
    }

    void LogWidget::onClickedAtColorRegexList (QModelIndex idx)
    {
        if (!idx.isValid()) return;
        QStandardItemModel * model = static_cast<QStandardItemModel *>(m_config_ui.ui()->listViewColorRegex->model());
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
        onInvalidateFilter();
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
        QStandardItem * root = static_cast<QStandardItemModel *>(m_config_ui.ui()->listViewColorRegex->model())->invisibleRootItem();
        QStandardItem * child = findChildByText(root, qItem);
        if (child == 0)
        {
            QList<QStandardItem *> row_items = addRow(qItem, false);
            root->appendRow(row_items);

            appendToColorRegex(qItem);
        }
        recompileColorRegexps();
    }

    void LogWidget::onColorRegexRm ()
    {
        QStandardItemModel * model = static_cast<QStandardItemModel *>(m_config_ui.ui()->listViewColorRegex->model());
        QModelIndex const idx = m_config_ui.ui()->listViewColorRegex->currentIndex();
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

    /*void Connection::setupModelColorRegex ()
    {
        if (!m_color_regex_model)
            m_color_regex_model = new QStandardItemModel;
        m_main_window->getWidgetColorRegex()->setModel(m_color_regex_model);
    }

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



