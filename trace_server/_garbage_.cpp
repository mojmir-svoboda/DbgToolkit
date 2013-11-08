
	// Clear
	/*QMenu * clearMenu = menuBar()->addMenu(tr("&Clear"));
	clearMenu->addAction(tr("Clear current table view"), m_server, SLOT(onClearCurrentView()), QKeySequence(Qt::Key_C));
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_L), this, SLOT(onClearCurrentView()));
	clearMenu->addAction(tr("Clear current file filter"), m_server, SLOT(onClearCurrentFileFilter()));
	clearMenu->addAction(tr("Clear current context filter"), m_server, SLOT(onClearCurrentCtxFilter()));
	clearMenu->addAction(tr("Clear current thread id filter"), m_server, SLOT(onClearCurrentCtxFilter()));
	clearMenu->addAction(tr("Clear current colorized regexp filter"), m_server, SLOT(onClearCurrentColorizedRegexFilter()));
	clearMenu->addAction(tr("Clear current regexp filter"), m_server, SLOT(onClearCurrentRegexFilter()));
	clearMenu->addAction(tr("Clear current collapsed scope filter"), m_server, SLOT(onClearCurrentScopeFilter()));
	clearMenu->addAction(tr("Clear current ref time"), m_server, SLOT(onClearCurrentRefTime()));*/

	//void onFileColOrExp (QModelIndex const & idx, bool collapsed);
	//void onFileExpanded (QModelIndex const & idx);
	//void onFileCollapsed (QModelIndex const & idx);
/*void LogWidget::onFileExpanded (QModelIndex const & idx)
{
	onFileColOrExp(idx, false);
}

void LogWidget::onFileCollapsed (QModelIndex const & idx)
{
	onFileColOrExp(idx, true);
}*/


/*
void MainWindow::storeState ()
{
	qDebug("%s", __FUNCTION__);

	//settings.setValue("splitter", ui->splitter->saveState());
	settings.setValue("autoScrollCheckBox", ui->autoScrollCheckBox->isChecked());
	settings.setValue("inViewCheckBox", ui->inViewCheckBox->isChecked());
	settings.setValue("filterFileCheckBox", ui->filterFileCheckBox->isChecked());
	settings.setValue("clrFiltersCheckBox", ui_settings->clrFiltersCheckBox->isChecked());
	//settings.setValue("filterModeComboBox", ui->filterModeComboBox->currentIndex());
	//settings.setValue("filterPaneComboBox", ui_settings->filterPaneComboBox->currentIndex());

	settings.setValue("scopesCheckBox1", ui_settings->scopesCheckBox->isChecked());
	settings.setValue("indentCheckBox", ui_settings->indentCheckBox->isChecked());
	settings.setValue("cutPathCheckBox", ui_settings->cutPathCheckBox->isChecked());
	settings.setValue("cutNamespaceCheckBox", ui_settings->cutNamespaceCheckBox->isChecked());
	settings.setValue("indentSpinBox", ui_settings->indentSpinBox->value());
	settings.setValue("tableRowSizeSpinBox", ui_settings->tableRowSizeSpinBox->value());
	settings.setValue("tableFontComboBox", ui_settings->tableFontComboBox->currentText());
	settings.setValue("cutPathSpinBox", ui_settings->cutPathSpinBox->value());
	settings.setValue("cutNamespaceSpinBox", ui_settings->cutNamespaceSpinBox->value());
}

void MainWindow::loadState ()
{
	qDebug("%s", __FUNCTION__);
	m_config.m_app_names.clear();
	m_config.m_columns_setup.clear();
	m_config.m_columns_sizes.clear();
	m_config.loadSearchHistory();
	updateSearchHistory();

	QSettings settings("MojoMir", "TraceServer");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	int const pane_val = settings.value("filterPaneComboBox", 0).toInt();
	ui_settings->filterPaneComboBox->setCurrentIndex(pane_val);
	if (settings.contains("splitter"))
	{
		//ui->splitter->restoreState(settings.value("splitter").toByteArray());
		//ui->splitter->setOrientation(pane_val ? Qt::Vertical : Qt::Horizontal);
	}

	ui_settings->traceStatsCheckBox->setChecked(settings.value("trace_stats", true).toBool());

	ui->autoScrollCheckBox->setChecked(settings.value("autoScrollCheckBox", true).toBool());

	if (ui->autoScrollCheckBox->checkState() != Qt::Checked)
		ui->inViewCheckBox->setChecked(settings.value("inViewCheckBox", true).toBool());

	ui_settings->reuseTabCheckBox->setChecked(settings.value("reuseTabCheckBox", true).toBool());
	ui_settings->scopesCheckBox->setChecked(settings.value("scopesCheckBox1", true).toBool());
	ui_settings->indentCheckBox->setChecked(settings.value("indentCheckBox", true).toBool());
	ui_settings->cutPathCheckBox->setChecked(settings.value("cutPathCheckBox", true).toBool());
	ui_settings->cutNamespaceCheckBox->setChecked(settings.value("cutNamespaceCheckBox", true).toBool());

	ui_settings->indentSpinBox->setValue(settings.value("indentSpinBox", 2).toInt());
	ui_settings->cutPathSpinBox->setValue(settings.value("cutPathSpinBox", 1).toInt());
	ui_settings->cutNamespaceSpinBox->setValue(settings.value("cutNamespaceSpinBox", 1).toInt());

	ui->tableSlider->setValue(settings.value("tableSlider", 0).toInt());
	ui->plotSlider->setValue(settings.value("plotSlider", 0).toInt());
	ui->ganttSlider->setValue(settings.value("ganttSlider", 0).toInt());
	ui->filterFileCheckBox->setChecked(settings.value("filterFileCheckBox", true).toBool());
	ui->buffCheckBox->setChecked(settings.value("buffCheckBox", true).toBool());
	ui_settings->clrFiltersCheckBox->setChecked(settings.value("clrFiltersCheckBox", false).toBool());
	//ui->filterModeComboBox->setCurrentIndex(settings.value("filterModeComboBox").toInt());
	//@TODO: delete filterMode from registry if exists
	if (m_start_level == -1)
	{
		qDebug("reading saved level from cfg");
		ui->levelSpinBox->setValue(settings.value("levelSpinBox", 3).toInt());
	}
	else
	{
		qDebug("reading level from command line");
		ui->levelSpinBox->setValue(m_start_level);
	}

	ui_settings->tableRowSizeSpinBox->setValue(settings.value("tableRowSizeSpinBox", 18).toInt());
	//ui_settings->tableFontComboBox->setValue(settings.value("tableFontComboBox", "Verdana 8").toInt());

	read_list_of_strings(settings, "known-applications", "application", m_config.m_app_names);
	for (int i = 0, ie = m_config.m_app_names.size(); i < ie; ++i)
	{
		m_config.m_columns_setup.push_back(columns_setup_t());
		settings.beginGroup(tr("column_order_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "orders", "column", m_config.m_columns_setup.back());
		}
		settings.endGroup();
		
		m_config.m_columns_sizes.push_back(columns_sizes_t());
		settings.beginGroup(tr("column_sizes_%1").arg(m_config.m_app_names[i]));
		{
			int const size = settings.beginReadArray("sizes");
			for (int i = 0; i < size; ++i) {
				settings.setArrayIndex(i);
				m_config.m_columns_sizes.back().push_back(settings.value("column").toInt());
			}
			settings.endArray();
		}
		settings.endGroup();

		m_config.m_columns_align.push_back(columns_align_t());
		settings.beginGroup(tr("column_align_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "aligns", "column", m_config.m_columns_align.back());
		}
		settings.endGroup();

		if (m_config.m_columns_align.back().size() < m_config.m_columns_sizes.back().size())
			for (int i = 0, ie = m_config.m_columns_sizes.back().size(); i < ie; ++i)
				m_config.m_columns_align.back().push_back(QString("L"));

		m_config.m_columns_elide.push_back(columns_elide_t());
		settings.beginGroup(tr("column_elide_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "elides", "column", m_config.m_columns_elide.back());
		}
		settings.endGroup();

		if (m_config.m_columns_elide.back().size() < m_config.m_columns_sizes.back().size())
			for (int i = 0, ie = m_config.m_columns_sizes.back().size(); i < ie; ++i)
				m_config.m_columns_elide.back().push_back(QString("R"));
	}

	if (m_config.m_thread_colors.empty())
	{
		for (size_t i = Qt::white; i < Qt::transparent; ++i)
			m_config.m_thread_colors.push_back(QColor(static_cast<Qt::GlobalColor>(i)));
	}

	convertBloodyBollockyBuggeryRegistry();

#ifdef WIN32
	unsigned const hotkeyCode = settings.value("hotkeyCode").toInt();
	m_config.m_hotkey = hotkeyCode ? hotkeyCode : VK_SCROLL;
	DWORD const hotkey = m_config.m_hotkey;
	int mod = 0;
	UnregisterHotKey(getHWNDForWidget(this), 0);
	RegisterHotKey(getHWNDForWidget(this), 0, mod, LOBYTE(hotkey));
#endif

	loadPresets();
	QString const pname = settings.value("presetComboBox").toString();
	ui->presetComboBox->setCurrentIndex(ui->presetComboBox->findText(pname));

	ui->dockedWidgetsToolButton->setChecked(m_docked_widgets->isVisible());
	qApp->installEventFilter(this);
}
*/

			/*{
			 *
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetColorRegex()->model());
				if (model)
				{
					if (QStandardItem * root = model->invisibleRootItem())
					{
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
					else
						qWarning("cregexp - nonexistent root");
				}
				else
					qWarning("cregexp - nonexistent model");

			}

			{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetRegex()->model());
				if (model)
				{
					if (QStandardItem * root = model->invisibleRootItem())
					{
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
					else
						qWarning("regexp - nonexistent root");
				}
				else
					qWarning("regexp - nonexistent model");
			}

			{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetLvl()->model());
				if (model)
				{
					if (QStandardItem * root = model->invisibleRootItem())
					{
						std::sort(sessionState().m_lvl_filters.begin(), sessionState().m_lvl_filters.end());
						for (int i = 0; i < sessionState().m_lvl_filters.size(); ++i)
						{
							FilteredLevel & flt = sessionState().m_lvl_filters[i];
							appendToLvlWidgets(flt);
						}
					}
					else
						qWarning("lvl - nonexistent root");
				}
				else
					qWarning("lvl - nonexistent model");

			}
			{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetCtx()->model());
				if (model)
				{
					if (QStandardItem * root = model->invisibleRootItem())
					{
						for (int i = 0; i < sessionState().m_ctx_filters.size(); ++i)
						{
							FilteredContext & flt = sessionState().m_ctx_filters[i];
							appendToCtxWidgets(flt);
						}
					}
					else
						qWarning("ctx - nonexistent root");
				}
				else
					qWarning("ctx - nonexistent model");
			}
			{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetString()->model());
				if (model)
				{
					if (QStandardItem * root = model->invisibleRootItem())
						for (int i = 0; i < sessionState().m_filtered_strings.size(); ++i)
						{
							FilteredString & flt = sessionState().m_filtered_strings[i];
							appendToStringWidgets(flt);
						}
					else
						qWarning("str - nonexistent root");
				}
				else
					qWarning("str - nonexistent model");
			}*/


/*
void MainWindow::saveSession (SessionState const & s, QString const & preset_name) const
{
	qDebug("%s", __FUNCTION__);
	QString fname = getPresetFileName(m_config.m_appdir, preset_name);
	qDebug("store file=%s", fname.toStdString().c_str());
	saveSessionState(s, fname.toLatin1());
}

void MainWindow::storePresets ()
{
	qDebug("%s", __FUNCTION__);
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = findCurrentConnection();
	if (!conn) return;

	storePresetNames();

	for (int i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
		if (!m_config.m_preset_names.at(i).isEmpty())
			saveSession(conn->sessionState(), m_config.m_preset_names.at(i));
}

void MainWindow::saveCurrentSession (QString const & preset_name)
{
	qDebug("%s name=%s", __FUNCTION__, preset_name.toStdString().c_str());
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = findCurrentConnection();
	if (!conn) return;

	saveSession(conn->sessionState(), preset_name);
}
*/

/*bool MainWindow::loadSession (SessionState & s, QString const & preset_name)
{
	qDebug("%s name=%s", __FUNCTION__, preset_name.toStdString().c_str());
	QString fname = getPresetFileName(m_config.m_appdir, preset_name);
	qDebug("load file=%s", fname.toStdString().c_str());
	s.m_file_filters.clear();
	return loadSessionState(s, fname.toLatin1());
}
*/
/*int MainWindow::presetCandidates (QString const & appname, QStringList & candidates, bool & default_present)
{
	QStringList subdirs;
	if (int const n = findPresetsForApp(m_config.m_appdir, appname, subdirs))
	{
		default_present = false;
		QStringList candidates;
		foreach (QString const & s, subdirs)
		{
			QString test_preset_name = getAppName() + "/" + s;
			QString const cfg_fname = getDataTagFileName(getConfig().m_appdir, test_preset_name, preset_prefix, tag);
			if (existsFile(cfg_fname))
			{
				if (s == QString(g_defaultPresetName))
					default_present = true;
				candidates << test_preset_name;
			}
			appendPresetNoFocus(test_preset_name);
		}
	}
	return candidates.size();
}*/

//void setupColumns (QList<QString> * cs_template, columns_sizes_t * sizes , columns_align_t * ca_template, columns_elide_t * ce_template);
		//void setupColumnsCSV (QList<QString> * cs_template, columns_sizes_t * sizes , columns_align_t * ca_template, columns_elide_t * ce_template);

		//void onEditFindNext ();
		//void onEditFindPrev ();
		//void onQSearch (QString const & text);
		//void onQSearchEditingFinished ();
		//void setLastSearchIntoCombobox (QString const & txt);
		//void onFindAllButton ();
		//void onQFilterLineEditFinished ();
		//void appendToSearchHistory (QString const & str);
		//void updateSearchHistory ();
	//void findAllTexts (QString const & text);
	//void findText (QString const & text, tlv::tag_t tag);
	//void findText (QString const & text);
	//void findNext (QString const & text);
	//void findPrev (QString const & text);
	//void findTextInAllColumns (QString const & text, int from_row, int to_row, bool only_first);
	//bool matchTextInCell (QString const & text, int row, int col);
	//void endOfSearch ();
	//void findTextInColumn (QString const & text, int col, int from_row, int to_row);
	//void findTextInColumnRev (QString const & text, int col, int from_row, int to_row);
/*void LogWidget::onApplyColumnSetup ()
{
	qDebug("%s", __FUNCTION__);
	for (int i = 0; i < horizontalHeader()->count(); ++i)
	{
		//qDebug("column: %s", horizontalHeader()->text());
	}

	QMap<int, int> order;

	if (sessionState().m_app_idx == -1)
		sessionState().m_app_idx = m_main_window->m_config.m_columns_setup.size() - 1;
	
	columns_setup_t const & new_cs = m_main_window->getColumnSetup(sessionState().m_app_idx);

	for (int i = 0, ie = new_cs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = tlv::tag_for_name(new_cs.at(i).toStdString().c_str());
		if (tag != tlv::tag_invalid)
		{
			order[tag] = i;
		}
	}

	if (0 == sessionState().m_columns_setup_current)
	{
	}
	else
	{
		columns_setup_t const & old_cs = *sessionState().m_columns_setup_current;
	}
	//static_cast<TableView *>(m_table_view_widget)->setColumnOrder(order, m_session_state);
}
*/

/*void FilterState::setupColumns (QList<QString> * cs_template, columns_sizes_t * sizes , columns_align_t * ca_template, columns_elide_t * ce_template)
{
	m_columns_sizes = sizes;
	m_columns_setup_template = cs_template;
	m_columns_align_template = ca_template;
	m_columns_elide_template = ce_template;

	if (!m_columns_setup_current)
	{
		m_columns_setup_current = new QList<QString>();
	}
	else
	{
		m_columns_setup_current->clear();
	}

	m_tags2columns.clear();
	*m_columns_setup_current = *m_columns_setup_template;
	for (size_t i = 0, ie = cs_template->size(); i < ie; ++i)
	{
		size_t const tag_idx = tlv::tag_for_name(cs_template->at(i).toStdString().c_str());
		if (tag_idx != tlv::tag_invalid)
		{
			m_tags2columns.insert(tag_idx, static_cast<int>(i)); // column index is int in Qt toolkit
			//qDebug("FilterState::setupColumns col[%u] tag_idx=%u tag_name=%s", i, tag_idx, cs->at(i).toStdString().c_str());
		}
	}
}

void FilterState::setupColumnsCSV (QList<QString> * cs_template, columns_sizes_t * sizes
			, columns_align_t * ca_template, columns_elide_t * ce_template)
{
	m_columns_sizes = sizes;
	m_columns_setup_template = cs_template;
	m_columns_align_template = ca_template;
	m_columns_elide_template = ce_template;

	if (!m_columns_setup_current)
	{
		m_columns_setup_current = new QList<QString>();
	}
	else
	{
		m_columns_setup_current->clear();
	}

	m_tags2columns.clear();
	*m_columns_setup_current = *m_columns_setup_template;
}*/


/*void LogWidget::onEditFind ()
{
	bool ok;
	QString search = QInputDialog::getText(this, tr("Find"), tr("Text:"), QLineEdit::Normal, m_last_search, &ok);
	if (ok)
	{
		m_last_search = search;
		if (int const pos = ui->qSearchComboBox->findText(search) >= 0)
			ui->qSearchComboBox->setCurrentIndex(pos);
		else
		{
			ui->qSearchComboBox->addItem(search);
			ui->qSearchComboBox->setCurrentIndex(ui->qSearchComboBox->findText(search));
		}
		onQSearch(search);
	}
}

void LogWidget::onQSearch (QString const & text)
{
	appendToSearchHistory(text);
	//QString qcolumn = ui->qSearchColumnComboBox->currentText();
	//qDebug("onQSearch: col=%s text=%s", qcolumn.toStdString().c_str(), text.toStdString().c_str());
	//bool const search_all = (qcolumn == ".*");
	bool const search_all = true;
	if (search_all)
	{
		findText(text);
	}
}

void LogWidget::onQSearchEditingFinished ()
{
	//QString const text = ui->qSearchComboBox->currentText();
	//onQSearch(text);
}

void LogWidget::setLastSearchIntoCombobox (QString const & txt)
{
	//ui->qSearchComboBox->addItem(txt);
	//ui->qSearchComboBox->setCurrentIndex(ui->qSearchComboBox->findText(txt));
}

void LogWidget::onFindAllButton ()
{
	//QString const text = ui->qSearchComboBox->currentText();
	//findAllTexts(text);
}
*/
/*void LogWidget::onQFilterLineEditFinished ()
{
	if (ui->qFilterLineEdit->text().size() == 0)
		return;

	QString text = ui->qFilterLineEdit->text();
	appendToStringFilters(text, true, true);

	QStandardItemModel * model = static_cast<QStandardItemModel *>(getWidgetString()->model());
	QStandardItem * root = model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, text);
	if (!child)
	{
		QList<QStandardItem *> row_items = addTriRow(text, Qt::Checked, true);
		root->appendRow(row_items);
		child = findChildByText(root, text);
		child->setCheckState(Qt::Checked);
	}

	for (int i = 0, ie = ui->tabFilters->count(); i < ie; ++i)
	{
		if (ui->tabFilters->tabText(i) == "String")
		{
			ui->tabFilters->setCurrentIndex(i);
			break;
		}
	}

	recompileStrings();
}

void LogWidget::appendToSearchHistory (QString const & str)
{
	if (str.length() == 0)
		return;
	m_config.m_search_history.insert(str);
	m_config.saveSearchHistory();
	updateSearchHistory();
	ui->qSearchComboBox->setCurrentIndex(ui->qSearchComboBox->findText(str));
}

void LogWidget::updateSearchHistory ()
{
	ui->qSearchComboBox->clear();
	for (size_t i = 0, ie = m_config.m_search_history.size(); i < ie; ++i)
		ui->qSearchComboBox->addItem(m_config.m_search_history[i]);
}*/


	
	/*void LogWidget::moveSectionsAccordingTo (logs::LogConfig const & cfg)
	{

		QMap<int, int> perms;
		int const hn = horizontalHeader()->count();
		for (int hi = 0; hi < hn; ++hi)
		{
			int const currentVisualIndex = horizontalHeader()->visualIndex(hi);
			//if (currentVisualIndex != i)
			if (currentVisualIndex > -1 && currentVisualIndex < m_config.m_columns_setup.size())
			{
				QString val = m_config.m_columns_setup[hi];
				
				int const nn = cfg.m_columns_setup.size();
				for (int nj = 0; nj < nn; ++nj)
					if (val == cfg.m_columns_setup[nj] && hi != nj)
						perms.insert(hi, nj);

			}
		}

		QMapIterator<int, int> iter(perms);
		while (iter.hasNext())
		{
			iter.next();
			int const logical = iter.key();
			int const visual = iter.value();
			horizontalHeader()->moveSection(logical, visual);
		}
	}*/

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


