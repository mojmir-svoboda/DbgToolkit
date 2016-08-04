#include "logwidget.h"
#include "logfilterproxymodel.h"
#include "connection.h"
#include "mainwindow.h"
#include <utils/utils_openfilelinewith.h>
#include <utils/find_utils_table.h>

namespace logs {

bool LogWidget::handleAction (Action * a, E_ActionHandleType sync)
{
	switch (a->type())
	{
		case e_Close:
		{
			m_connection->destroyDockedWidget(this);
			setParent(0);
			delete this;
			return true;
		}

		case e_Visibility:
		{
			Q_ASSERT(a->m_args.size() > 0);
			bool const on = a->m_args.at(0).toBool();
			setVisible(on);
			m_connection->getMainWindow()->onDockRestoreButton();
			return true;
		}

		case e_Find:
		{
			if (a->m_args.size() > 0)
			{
				if (a->m_args.at(0).canConvert<FindConfig>())
				{
					FindConfig const fc = a->m_args.at(0).value<FindConfig>();
					handleFindAction(fc);
					m_config.m_find_config = fc;
					// m_config.save
				}
				return true;
			}
		}
		case e_QuickString:
		{
			if (a->m_args.size() > 0)
			{
				if (a->m_args.at(0).canConvert<QuickStringConfig>())
				{
					QuickStringConfig const fc = a->m_args.at(0).value<QuickStringConfig>();
					handleQuickStringAction(fc);
					m_config.m_quick_string_config = fc;
					// m_config.save
				}
				return true;
			}
		}
		case e_Colorize:
		{
			if (a->m_args.size() > 0)
			{
				if (a->m_args.at(0).canConvert<ColorizeConfig>())
				{
					ColorizeConfig const cc = a->m_args.at(0).value<ColorizeConfig>();
					handleColorizeAction(cc);
					m_config.m_colorize_config = cc;
					// m_config.save
				}
				return true;
			}
		}

		case e_ColorTagLastLine:
      onColorLastRow();
			return true;

		case e_ScrollToLastLine:
			autoScrollOn();
			m_tableview->scrollToBottom();
			return true;

    case e_DeleteAllData:
      onClearAllDataButton();
      return true;

		case e_SetRefSTime:
		{
			Q_ASSERT(a->m_args.size() > 0);
			unsigned long long const t = a->m_args.at(0).toULongLong();
			setSTimeRefValue(t);
			return true;
		}
		default:
			return false;
	}
	return false;
}

void LogWidget::onTableClicked (QModelIndex const & row_index)
{
	autoScrollOff();

	QModelIndex const curr_src_idx = currentSourceIndex();
	if (!curr_src_idx.isValid())
		return;

	bool const scroll_to_item = false;
	bool const expand = false;
	findTableIndexInFilters(curr_src_idx, scroll_to_item, expand);
}

void LogWidget::onTableFontToolButton ()
{
	/*
	bool ok = false;
	QFont curr_font;
	Connection * conn = m_server->findCurrentConnection();
	if (conn)
	{
		curr_font = conn->getTableViewWidget()->font();
		ui_settings->tableFontComboBox->addItem(curr_font.toString());
	}

	QFont f = QFontDialog::getFont(&ok, curr_font);
	if (ok)
	{
		ui_settings->tableFontComboBox->insertItem(0, curr_font.toString());
		verticalHeader()->setFont(f);
	}
	*/
}


void LogWidget::onNextToView ()
{
	nextToView();
}

void LogWidget::findTableIndexInFilters (QModelIndex const & src_index, bool scroll_to_item, bool expand)
{
	{
		QModelIndex const idx_f = m_src_model->index(src_index.row(), proto::tag2col<proto::int_<proto::tag_file>>::value, QModelIndex());
		QVariant const val_f = m_src_model->data(idx_f, Qt::DisplayRole);
		QString const str_f = val_f.toString();
		QModelIndex const idx_l = m_src_model->index(src_index.row(), proto::tag2col<proto::int_<proto::tag_line>>::value, QModelIndex());
		QVariant const val_l = m_src_model->data(idx_l, Qt::DisplayRole);
		QString str_l = QString::number(val_l.toULongLong());

		QString FIXME = str_f + "/" + str_l;
		if (filterMgr()->getFilterFileLine())
			filterMgr()->getFilterFileLine()->locateItem(FIXME, scroll_to_item, expand);
	}
	{
		QModelIndex const idx_t = m_src_model->index(src_index.row(), proto::tag2col<proto::int_<proto::tag_tid>>::value, QModelIndex());
		QVariant const val_t = m_src_model->data(idx_t, Qt::DisplayRole);
		QString str_t = QString::number(val_t.toULongLong());
		if (filterMgr()->getFilterTid())
			filterMgr()->getFilterTid()->locateItem(str_t, scroll_to_item, expand);;
	}
// 	{
// 		QString lvl;
// 		if (dcmd->getString(proto::tag_lvl, lvl))
// 		{
// 			if (filterMgr()->getFilterLvl())
// 				filterMgr()->getFilterLvl()->locateItem(lvl, scroll_to_item, expand);;
// 		}
// 		QString ctx;
// 		if (dcmd->getString(proto::tag_ctx, ctx))
// 		{
// 			if (filterMgr()->getFilterCtx())
// 				filterMgr()->getFilterCtx()->locateItem(ctx, scroll_to_item, expand);;
// 		}
// 	}
}

void LogWidget::colorRow (int)
{
	QModelIndex curr_src_idx = currentSourceIndex();

	int const row = curr_src_idx.row(); // set search from this line
	if (curr_src_idx.isValid())
	{
		qDebug("Color tag on row=%i", curr_src_idx.row());
		addColorTagRow(curr_src_idx.row());
		onInvalidateFilter();
	}
}

/*void LogWidget::onClearCurrentView ()
{
	LogTableModel * model = static_cast<LogTableModel *>(m_proxy_model ? m_proxy_model->sourceModel() : m_src_model);
	//excludeContentToRow(model->rowCount());
	//onInvalidateFilter();
}*/

void LogWidget::excludeFile (QModelIndex const & src_index)
{
	if (filterMgr()->getFilterFileLine())
	{
		if (!src_index.isValid())
			return;

		QModelIndex const idx_f = m_src_model->index(src_index.row(), proto::tag2col<proto::int_<proto::tag_file>>::value, QModelIndex());
		QVariant const val_f = m_src_model->data(idx_f, Qt::DisplayRole);
		QString const str_f = val_f.toString();
		QModelIndex const result = filterMgr()->getFilterFileLine()->fileModel()->stateToItem(str_f, Qt::Unchecked);
		if (!result.isValid())
		{
			qWarning("%s: nonexistent index!", __FUNCTION__);
		}

		onInvalidateFilter();
	}
}

void LogWidget::excludeSelectionRows (void (LogWidget::*fn)(QModelIndex const & idx))
{
	int const col = proto::tag2col<proto::int_<proto::tag_file>>::value;

	std::vector<QModelIndex> l;
	l.reserve(1024); // @TODO: perf

	uniqueRowsFromCurrSelection(m_tableview, l);

	for (QModelIndex const & index : l)
	{
		QModelIndex src_index = index;
		if (isModelProxy())
		{
			src_index = m_proxy_model->mapToSource(index);
		}

		if (col != index.column())
			src_index = m_src_model->index(src_index.row(), col, QModelIndex());

		if (src_index.isValid())
		{
			(this->*fn)(src_index);
		}
	}
}

void LogWidget::onExcludeFileLine ()
{
	excludeSelectionRows(&LogWidget::excludeFileLine);
}
void LogWidget::onExcludeFile ()
{
	excludeSelectionRows(&LogWidget::excludeFile);
}

void LogWidget::excludeFileLine (QModelIndex const & src_index)
{
	if (filterMgr()->getFilterFileLine())
	{
		if (!src_index.isValid())
			return;

		QModelIndex const idx_f = m_src_model->index(src_index.row(), proto::tag2col<proto::int_<proto::tag_file>>::value, QModelIndex());
		QVariant const val_f = m_src_model->data(idx_f, Qt::DisplayRole);
		QString const str_f = val_f.toString();
		QModelIndex const idx_l = m_src_model->index(src_index.row(), proto::tag2col<proto::int_<proto::tag_line>>::value, QModelIndex());
		QVariant const val_l = m_src_model->data(idx_l, Qt::DisplayRole);
		QString str_l = QString::number(val_l.toULongLong());

		QString FIXME = str_f + "/" + str_l;

		QModelIndex const result = filterMgr()->getFilterFileLine()->fileModel()->stateToItem(FIXME, Qt::Unchecked);
		if (!result.isValid())
		{
			qWarning("%s: nonexistent index!", __FUNCTION__);
		}

		onInvalidateFilter();
	}
}

void LogWidget::excludeRow (QModelIndex const & src_index)
{
	if (!filterMgr()->getFilterRow())
	{
		// request to create filter
	}

	if (filterMgr()->getFilterRow())
	{
		if (!src_index.isValid())
			return;

		qDebug("excluding: %i", src_index.row());
		FilteredRow cfg(src_index.row(), true, e_CmpEQ);
		filterMgr()->getFilterRow()->addFilteredRow(cfg);
	}
}

void LogWidget::onExcludeRow ()
{
	excludeSelectionRows(&LogWidget::excludeRow);
}
void LogWidget::onLocateRow ()
{
	QModelIndex curr_src_idx = currentSourceIndex();
	if (!curr_src_idx.isValid())
		return;

	bool const scroll_to_item = true;
	bool const expand = true;
	findTableIndexInFilters(curr_src_idx, scroll_to_item, expand);
	filterMgr()->focusToFilter(e_Filter_FileLine);
	m_config_ui->ui()->groupingWidget->setCurrentWidget(m_config_ui->ui()->filtersPage);
	// @TODO: locate in colorizer row
}

void LogWidget::onColorFileLine ()
{
}
void LogWidget::onColorRow ()
{
	colorRow(0);
}
void LogWidget::onUncolorRow ()
{
	QModelIndex curr_src_idx = currentSourceIndex();

	int const row = curr_src_idx.row(); // set search from this line
	if (curr_src_idx.isValid())
	{
		qDebug("uncolor tag on row=%i", curr_src_idx.row());
		removeColorTagRow(curr_src_idx.row());
		onInvalidateFilter();
	}
}

void LogWidget::onChangeTimeUnits (int)
{
	QString const & curr = m_timeComboBox->comboBox()->currentText();
	float const t = stringToUnitsValue(curr);
	m_config.m_time_units_str = curr;
	m_config.m_time_units = t;
	onInvalidateFilter(); // TODO: invalidate only column?
}

void LogWidget::onSetRefCTime ()
{
	if (m_setRefCTimeButton->isChecked())
	{
		QModelIndex const curr_src_idx = currentSourceIndex();
		if (curr_src_idx.isValid())
		{
			QModelIndex const tag_idx = m_src_model->index(curr_src_idx.row(), proto::tag2col<proto::int_<proto::tag_ctime>>::value, QModelIndex());
			QVariant const tag_var = m_src_model->data(tag_idx, Qt::DisplayRole);
			setCTimeRefValue(tag_var.toULongLong());
			onInvalidateFilter();
		}
	}
	else
	{
		clearRefCTime();
		onInvalidateFilter(); // TODO: invalidate only column?
	}
}

void LogWidget::onSetRefSTime()
{
	if (m_setRefSTimeButton->isChecked())
	{
		QModelIndex const curr_src_idx = currentSourceIndex();
		if (curr_src_idx.isValid())
		{
			QModelIndex const tag_idx = m_src_model->index(curr_src_idx.row(), proto::tag2col<proto::int_<proto::tag_stime>>::value, QModelIndex());
			QVariant const tag_var = m_src_model->data(tag_idx, Qt::DisplayRole);
			uint64_t const t = tag_var.toULongLong();
			emitRequestSynchronization(e_SyncRefSTime, m_config.m_sync_group, t, this); // this is ignored in the call
			setSTimeRefValue(t);
			onInvalidateFilter();
		}
	}
	else
	{
		emitRequestSynchronization(e_SyncRefSTime, m_config.m_sync_group, 0, this); // this is ignored in the call
		clearRefSTime();
		onInvalidateFilter(); // TODO: invalidate only column?
	}
}

void LogWidget::onColorLastRow ()
{
	QModelIndex bottom = m_tableview->model()->index(m_tableview->model()->rowCount() - 1, 0);

	if (isModelProxy())
	{
		QModelIndex bottom_in_src = m_proxy_model->mapToSource(bottom);
		addColorTagRow(bottom_in_src.row());
	}
  else
		addColorTagRow(bottom.row());

	//onInvalidateFilter();
}

void LogWidget::onGotoPrevErr ()
{
	FindConfig fc;
	fc.m_next = 0;
	fc.m_prev = 1;
	fc.m_select = 1;
	fc.m_refs = 0;
	fc.m_clone = 0;
	fc.m_case_sensitive = 0;
	fc.m_whole_word = 0;
	fc.m_regexp = 0;
	fc.m_str = "Error";
	if (!fc.m_where.hasValidConfig())
		fillDefaultConfigWithLogTags(fc.m_where);
	findAndSelectPrev(m_tableview, fc);
}
void LogWidget::onGotoNextErr ()
{
	FindConfig fc;
	fc.m_next = 1;
	fc.m_prev = 0;
	fc.m_select = 1;
	fc.m_refs = 0;
	fc.m_clone = 0;
	fc.m_case_sensitive = 0;
	fc.m_whole_word = 0;
	fc.m_regexp = 0;
	fc.m_str = "Error";
	if (!fc.m_where.hasValidConfig())
		fillDefaultConfigWithLogTags(fc.m_where);
	findAndSelectNext(m_tableview, fc);
}
void LogWidget::onGotoPrevWarn ()
{
	FindConfig fc;
	fc.m_next = 0;
	fc.m_prev = 1;
	fc.m_select = 1;
	fc.m_refs = 0;
	fc.m_clone = 0;
	fc.m_case_sensitive = 0;
	fc.m_whole_word = 0;
	fc.m_regexp = 0;
	fc.m_str = "Warning";
	if (!fc.m_where.hasValidConfig())
		fillDefaultConfigWithLogTags(fc.m_where);
	findAndSelectPrev(m_tableview, fc);
}
void LogWidget::onGotoNextWarn ()
{
	FindConfig fc;
	fc.m_next = 1;
	fc.m_prev = 0;
	fc.m_select = 1;
	fc.m_refs = 0;
	fc.m_clone = 0;
	fc.m_case_sensitive = 0;
	fc.m_whole_word = 0;
	fc.m_regexp = 0;
	fc.m_str = "Warning";
	if (!fc.m_where.hasValidConfig())
		fillDefaultConfigWithLogTags(fc.m_where);
	findAndSelectNext(m_tableview, fc);
}

void LogWidget::onHidePrev ()
{
	QModelIndex curr_src_idx = currentSourceIndex();
	if (!curr_src_idx.isValid())
		return;

	if (!filterMgr()->getFilterRow())
	{
		// request to create filter
	}

	if (filterMgr()->getFilterRow())
	{
		qDebug("excluding: %i", curr_src_idx.row());
		FilteredRow cfg(curr_src_idx.row(), true, e_CmpL);
		FilteredRow & f = filterMgr()->getFilterRow()->findOrCreateFilteredRow(QString::number(curr_src_idx.row()));
		f = cfg;
		filterMgr()->getFilterRow()->addFilteredRow(f);
	}
}
void LogWidget::onHideNext () //@TODO: dedup
{
	QModelIndex curr_src_idx = currentSourceIndex();
	if (!curr_src_idx.isValid())
		return;

	if (!filterMgr()->getFilterRow())
	{
		// request to create filter
	}

	if (filterMgr()->getFilterRow())
	{
		qDebug("excluding: %i", curr_src_idx.row());
		FilteredRow cfg(curr_src_idx.row(), true, e_CmpG);
		FilteredRow & f = filterMgr()->getFilterRow()->findOrCreateFilteredRow(QString::number(curr_src_idx.row()));
		f = cfg;
		filterMgr()->getFilterRow()->addFilteredRow(f);
	}
}


void LogWidget::onTableDoubleClicked (QModelIndex const & row_index)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
		if (m_config.m_sync_group == 0)
			return;	// do not sync groups with zero
	
		E_SyncMode const mode = e_SyncServerTime;
		//@TODO: mode from UI
	
		QModelIndex curr_src_idx = row_index;
		if (isModelProxy())
		{
			curr_src_idx = m_proxy_model->mapToSource(row_index);
		}
	
		unsigned long long time = 0;
		if (mode == e_SyncServerTime)
		{
			QModelIndex const tag_idx = m_src_model->index(curr_src_idx.row(), proto::tag2col<proto::int_<proto::tag_stime>>::value, QModelIndex());
			QVariant const tag_var = m_src_model->data(tag_idx, Qt::DisplayRole);
			time = tag_var.toULongLong();
		}
		else
		{
			QModelIndex const tag_idx = m_src_model->index(curr_src_idx.row(), proto::tag2col<proto::int_<proto::tag_ctime>>::value, QModelIndex());
			QVariant const tag_var = m_src_model->data(tag_idx, Qt::DisplayRole);
			time = tag_var.toULongLong();
		}
	
		qDebug("table: dblClick curr=(%i, %i)  time=%llu", curr_src_idx.row(), curr_src_idx.column(), time);
		emit requestSynchronization(mode, m_config.m_sync_group, time, this);
	
	/*	if (m_proxy_model)
		{
			QModelIndex const curr = m_proxy_model->mapToSource(row_index);
			LogTableModel * model = static_cast<LogTableModel *>(m_proxy_model ? m_proxy_model->sourceModel() : model());
			qDebug("2c curr: (%i,col) -> (%i,col)", row_index.row(), curr.row());
	
			int row_bgn = curr.row();
			int row_end = curr.row();
			int layer = model->layers()[row_bgn];
	
			if (model->rowTypes()[row_bgn] == proto::cmd_scope_exit)
			{
				layer += 1;
				// test na range
				--row_bgn;
			}
	
			QString tid = findString4Tag(proto::tag_tid, curr);
			QString file = findString4Tag(proto::tag_file, curr);
			QString line = findString4Tag(proto::tag_line, curr);
			int from = row_bgn;
	
			if (model->rowTypes()[from] != proto::cmd_scope_entry)
			{
				while (row_bgn > 0)
				{
					QModelIndex const curr_idx = model->index(row_bgn, row_index.column(), QModelIndex());
					QString curr_tid = findString4Tag(proto::tag_tid, curr_idx);
					if (curr_tid == tid)
					{
						if (static_cast<int>(model->layers()[row_bgn]) >= layer)
						{
							from = row_bgn;
						}
						else
						{	
							break;
						}
					}
					--row_bgn;
				}
			}
	
			int to = row_end;
			if (model->rowTypes()[to] != proto::cmd_scope_exit)
			{
				while (row_end < static_cast<int>(model->layers().size()))
				{
					QModelIndex const curr_idx = model->index(row_end, row_index.column(), QModelIndex());
					QString next_tid = findString4Tag(proto::tag_tid, curr_idx);
					if (next_tid == tid)
					{
						if (model->layers()[row_end] >= layer)
							to = row_end;
						else if ((model->layers()[row_end] == layer - 1) && (model->rowTypes()[row_end] == proto::cmd_scope_exit))
						{
							to = row_end;
							break;
						}
						else
							break;
					}
					++row_end;
				}
			}
	
			qDebug("row=%u / curr=%u layer=%u, from,to=(%u, %u)", row_index.row(), curr.row(), layer, from, to);
			if (m_session_state.findCollapsedBlock(tid, from, to))
				m_session_state.eraseCollapsedBlock(tid, from, to);
			else
				m_session_state.appendCollapsedBlock(tid, from, to, file, line);
			onInvalidateFilter();
		}*/
}

void LogWidget::onOpenFileLine ()
{
	QModelIndex const curr_src_idx = currentSourceIndex();
	if (!curr_src_idx.isValid())
		return;

// 	DecodedCommand const * dcmd = getDecodedCommand(curr_src_idx);
// 	if (dcmd)
// 	{
// 		QString file, line;
// 		if (dcmd->getString(proto::tag_file, file) && dcmd->getString(proto::tag_line, line))
// 		{
// 			openFileLineWith(m_config_ui->ui()->openFileWithComboBox->currentText(), file, line);
// 		}
// 	}
}

void LogWidget::onSyncGroupRowButton ()
{
	QModelIndex curr_src_idx = currentSourceIndex();
	if (!curr_src_idx.isValid())
		return;

	onTableDoubleClicked(curr_src_idx);
}

void LogWidget::onSyncGroupValueChanged (int i)
{
	m_config.m_sync_group = i;
}

bool LogWidget::processBySounds (QModelIndex const & sourceIndex)
{
	return soundMgr()->action(sourceIndex);
}



}

