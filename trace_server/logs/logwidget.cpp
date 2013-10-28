#include "logwidget.h"
//#include <QScrollBar>
//#include <QSplitter>
#include <connection.h>
#include <utils.h>
#include <utils_qstandarditem.h>
#include "logdelegate.h"
#include <syncwidgets.h>
#include "logtablemodel.h"
#include "filterproxymodel.h"
#include "findproxymodel.h"
#include <QInputDialog>
#include <QFontDialog>


namespace logs {

	LogWidget::LogWidget (Connection * connection, QWidget * wparent, LogConfig & cfg, QString const & fname, QStringList const & path)
		: TableView(wparent), ActionAble(path)
		, m_config(cfg)
		, m_config2(cfg)
		, m_config_ui(*this, this)
		, m_fname(fname)
		, m_connection(connection)
		, m_tab(0)
		, m_linked_parent(0)
		, m_filter_state()
		, m_tagconfig()
		, m_tags2columns()
		, m_tls()
		, m_last_search_row(0)
		, m_last_search_col(0)
		, m_last_search()
		, m_column_setup_done(false)
		, m_exclude_content_to_row(0)
		, m_time_ref_row(0)
		, m_color_tag_rows()
		, m_current_tag(-1)
		, m_current_selection(-1)
		, m_time_ref_value(0)
		, m_curr_preset()
		, m_proxy_model(0)
		, m_find_proxy_model(0)
		, m_src_model(0)
		, m_selection(0)
		, m_kselection_model(0)
		, m_src_selection(0)
		, m_proxy_selection(0)
		, m_find_proxy_selection(0)
		, m_ctx_menu()
		, m_actions()
		, m_last_clicked()
		, m_csv_separator()
		, m_file_csv_stream(0)
		//, m_file_tlv_stream(0)
	{
		m_queue.reserve(256);

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		setConfigValuesToUI(m_config);
		//setUpdatesEnabled(true);
		horizontalHeader()->setSectionsMovable(true);
		//setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

		connect(&getSyncWidgets(), SIGNAL( requestTimeSynchronization(int, unsigned long long, void *) ),
							 this, SLOT( performTimeSynchronization(int, unsigned long long, void *) ));
		connect(this, SIGNAL( requestTimeSynchronization(int, unsigned long long, void *) ),
							 &getSyncWidgets(), SLOT( performTimeSynchronization(int, unsigned long long, void *) ));
		connect(&getSyncWidgets(), SIGNAL( requestFrameSynchronization(int, unsigned long long, void *) ),
							 this, SLOT( performFrameSynchronization(int, unsigned long long, void *) ));
		connect(this, SIGNAL( requestFrameSynchronization(int, unsigned long long, void *) ),
							 &getSyncWidgets(), SLOT( performFrameSynchronization(int, unsigned long long, void *) ));

		setStyleSheet("QTableView::item{ selection-background-color:	#F5DEB3  } QTableView::item{ selection-color:	#000000 }");
		
		// to ignore 'resizeColumnToContents' when accidentaly double-clicked on header handle
		QObject::disconnect(horizontalHeader(), SIGNAL(sectionHandleDoubleClicked(int)), this, SLOT(resizeColumnToContents(int)));

		setObjectName(QString::fromUtf8("tableView"));
		//setupThreadColors(connection->getMainWindow()->getThreadColors());


		QStyle const * const style = QApplication::style();
		m_config_ui.ui()->findWidget->setMainWindow(m_connection->getMainWindow());
		connect(m_config_ui.ui()->gotoNextButton, SIGNAL(clicked()), this, SLOT(onNextToView()));
		m_config_ui.ui()->gotoNextButton->setIcon(style->standardIcon(QStyle::SP_ArrowDown));
		//connect(ui->autoScrollCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onAutoScrollStateChanged(int)));
		//connect(ui->inViewCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onInViewStateChanged(int)));
		//
		//
		//connect(m_config_ui.ui()->filterFileCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onFilterFile(int)));

		QObject::connect(horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));
		QObject::connect(horizontalHeader(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(onSectionMoved(int, int, int)));
		verticalHeader()->setFont(cfg.m_font);
		verticalHeader()->setDefaultSectionSize(cfg.m_row_width);
		verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance
		horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		setItemDelegate(new LogDelegate(*this, m_connection->appData(), this));


		/*m_actions.resize(e_action_max_enum_value);
		m_actions[e_action_ToggleRef] = new QAction("Set as reference time", this);
		m_actions[e_action_HidePrev] = new QAction("Hide prev rows", this);
		m_actions[e_action_ExcludeFileLine] = new QAction("Exclude File:Line (x)", this);
		m_actions[e_action_Copy] = new QAction("Copy", this);
		m_actions[e_action_Find] = new QAction("Find File:Line in filters", this);
		m_actions[e_action_ColorTag] = new QAction("Tag row with color", this);
		m_actions[e_action_Setup] = new QAction("Setup", this);
		m_ctx_menu.addAction(m_actions[e_action_ExcludeFileLine]);
		m_ctx_menu.addAction(m_actions[e_action_Find]);
		m_ctx_menu.addAction(m_actions[e_action_Copy]);
		m_ctx_menu.addAction(m_actions[e_action_ToggleRef]);
		m_ctx_menu.addAction(m_actions[e_action_HidePrev]);
		m_ctx_menu.addAction(m_actions[e_action_ColorTag]);
		m_ctx_menu.addSeparator();
		m_ctx_menu.addAction(m_actions[e_action_Setup]);*/

		//m_data_model = new TreeModel(this, &m_session_state.m_data_filters);
		
		//applyConfig(m_config);
	}

	void LogWidget::setupNewLogModel ()
	{
		setupLogModel(0);
	}

	void LogWidget::setupLogModel (LogTableModel * linked_model)
	{
		LogTableModel * model = 0;
		if (linked_model)
			model = linked_model;
		else
		{
			model = new LogTableModel(this, *this);
			QObject::disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), verticalHeader(), SLOT(sectionsInserted(QModelIndex,int,int)));
		}

		m_src_model = model;

		//m_proxy_selection = new QItemSelectionModel(m_proxy_model);
		setupLogSelectionProxy();

		m_proxy_model = new FilterProxyModel(this, *this);
		m_proxy_model->setSourceModel(m_src_model);

		m_find_proxy_model = new FindProxyModel(this, *this);
		m_find_proxy_model->setSourceModel(m_src_model);
	}

	void LogWidget::setupLogSelectionProxy ()
	{
		m_src_selection = new QItemSelectionModel(m_src_model);
		m_proxy_selection = new QItemSelectionModel(m_proxy_model);
		//setSelectionModel(m_src_selection);
		//m_selection = new LogSelectionProxyModel(m_src_model, m_src_selection);
		//m_kselection_model = new KLinkItemSelectionModel(m_src_model, m_);
	}

	LogWidget::~LogWidget ()
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

/*	if (m_file_csv_stream)
	{
		QIODevice * const f = m_file_csv_stream->device();
		f->close();
		delete m_file_csv_stream;
		m_file_csv_stream = 0;
		delete f;
	}

	destroyModelFile();

	if (m_main_window->getWidgetLvl()->itemDelegate() == m_delegates.get<e_delegate_Level>())
		m_main_window->getWidgetLvl()->setItemDelegate(0);
	if (m_main_window->getWidgetLvl()->model() == m_lvl_model)
		m_main_window->getWidgetLvl()->setModel(0);
	delete m_lvl_model;
	m_lvl_model = 0;
	delete m_delegates.get<e_delegate_Level>();
	m_delegates.get<e_delegate_Level>() = 0;

	if (m_main_window->getWidgetCtx()->itemDelegate() == m_delegates.get<e_delegate_Ctx>())
		m_main_window->getWidgetCtx()->setItemDelegate(0);
	if (m_main_window->getWidgetCtx()->model() == m_ctx_model)
		m_main_window->getWidgetCtx()->setModel(0);
	delete m_ctx_model;
	m_ctx_model = 0;
	delete m_delegates.get<e_delegate_Ctx>();
	m_delegates.get<e_delegate_Ctx>() = 0;

	if (m_main_window->getWidgetTID()->model() == m_tid_model)
		m_main_window->getWidgetTID()->setModel(0);
	delete m_tid_model;
	m_tid_model = 0;

	if (m_main_window->getWidgetColorRegex()->model() == m_color_regex_model)
		m_main_window->getWidgetColorRegex()->setModel(0);
	delete m_color_regex_model;
	m_color_regex_model = 0;

	if (m_main_window->getWidgetRegex()->itemDelegate() == m_delegates.get<e_delegate_Regex>())
		m_main_window->getWidgetRegex()->setItemDelegate(0);
	if (m_main_window->getWidgetRegex()->model() == m_regex_model)
		m_main_window->getWidgetRegex()->setModel(0);
	delete m_regex_model;
	m_regex_model = 0;
	delete m_delegates.get<e_delegate_Regex>();
	m_delegates.get<e_delegate_Regex>() = 0;

	if (m_main_window->getWidgetString()->itemDelegate() == m_delegates.get<e_delegate_String>())
		m_main_window->getWidgetString()->setItemDelegate(0);
	if (m_main_window->getWidgetString()->model() == m_string_model)
		m_main_window->getWidgetString()->setModel(0);
	delete m_string_model;
	m_string_model = 0;
	delete m_delegates.get<e_delegate_String>();
	m_delegates.get<e_delegate_String>() = 0;

	if (m_proxy_model)
	{
		m_proxy_model->setSourceModel(0);
		delete m_proxy_model;
		m_proxy_model = 0;
	}

	setModel(0);
	delete m_src_model;
	m_src_model = 0;
	*/

	}

	void LogWidget::onShow ()
	{
		show();
	}

	void LogWidget::onHide ()
	{
		hide();
	}

	void LogWidget::onHideContextMenu ()
	{
		Ui::SettingsLog * ui = m_config_ui.ui();
		disconnect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		disconnect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		m_config_ui.onHideContextMenu();
	}

	void LogWidget::onShowContextMenu (QPoint const & pos)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		m_config_ui.onShowContextMenu(QCursor::pos());
		Ui::SettingsLog * ui = m_config_ui.ui();

		setConfigValuesToUI(m_config);
		connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		//connect(ui->logViewComboBox, SIGNAL(activated(int)), this, SLOT(onLogViewActivate(int)));
	}
	
	void LogWidget::moveSectionsAccordingTo (logs::LogConfig const & cfg)
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
	}


	void LogWidget::applyConfig ()
	{
		filterMgr()->disconnectFiltersTo(this);
		moveSectionsAccordingTo(m_config);
		//m_config = m_config2;
		applyConfig(m_config);
		filterMgr()->applyConfig();
		filterMgr()->connectFiltersTo(this);

		connect(filterMgr(), SIGNAL(filterEnabledChanged()), this, SLOT(onFilterEnabledChanged()));
		connect(filterMgr(), SIGNAL(filterChangedSignal()), this, SLOT(onInvalidateFilter()));

		if (filterMgr()->getFilterCtx())
			filterMgr()->getFilterCtx()->setAppData(&m_connection->appData());
	}

	void LogWidget::resizeSections ()
	{
		bool const old = blockSignals(true);
		for (int c = 0, ce = m_config.m_columns_sizes.size(); c < ce; ++c)
			horizontalHeader()->resizeSection(c, m_config.m_columns_sizes.at(c));
		blockSignals(old);
	}

	void LogWidget::applyConfig (LogConfig & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		//Ui::SettingsLog * ui = m_config_ui.ui();
		//horizontalHeader()->resizeSections(QHeaderView::Fixed);
		//horizontalHeader()->resizeSectionItem(c, 32, );

		if (m_src_model)
			m_src_model->resizeToCfg();
		if (m_proxy_model)
			m_proxy_model->resizeToCfg();
		if (m_find_proxy_model)
			m_find_proxy_model->resizeToCfg();

		if (!isLinkedWidget())
			setFilteringProxy(filterMgr()->enabled());

		resizeSections();
	}

	int LogWidget::sizeHintForColumn (int column) const
	{
		/*if (idx < m_config.m_columns_setup.size())
		{
			m_config.m_columns_sizes[idx] = new_size;
		}*/

		//@TODO: proxy !!!!!!!!!!!!!!!!!!
		int const idx = !isModelProxy() ? column : m_proxy_model->colToSource(column);
		//qDebug("table: on rsz hdr[%i -> src=%02i ]  %i->%i\t\t%s", c, idx, old_size, new_size, m_config.m_hhdr.at(idx).toStdSt
		if (idx < 0) return 64;
		int const curr_sz = m_config.m_columns_sizes.size();
		if (idx < curr_sz)
		{
			//qDebug("%s this=0x%08x hsize[%i]=%i", __FUNCTION__, this, idx, new_size);
			return m_config.m_columns_sizes[idx];
		}
		return 32;
	}

	void LogWidget::setConfigValuesToUI (LogConfig const & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsLog * ui = m_config_ui.ui();
		
		//ui->globalShowCheckBox->blockSignals(true);
		//ui->globalShowCheckBox->setCheckState(cfg.m_show ? Qt::Checked : Qt::Unchecked);
		//ui->globalShowCheckBox->blockSignals(false);

		//ui->logViewComboBox->clear();
		/*for (size_t i = 0, ie = cfg.m_gvcfg.size(); i < ie; ++i)
		{
			LogViewConfig const & gvcfg = cfg.m_gvcfg[i];
			ui->logViewComboBox->addItem(gvcfg.m_tag);
		}
		if (cfg.m_gvcfg.size())
			setViewConfigValuesToUI(cfg.m_gvcfg[0]);*/
	}

	void LogWidget::setUIValuesToConfig (LogConfig & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsLog * ui = m_config_ui.ui();
		//m_config.m_show = ui->globalShowCheckBox->checkState() == Qt::Checked;
	}

	void LogWidget::onApplyButton ()
	{
		applyConfig();
		//setUIValuesToConfig(m_config2);
		//applyConfig();
	}

	QString LogWidget::getCurrentPresetPath ()
	{
		QString const appdir = m_connection->getMainWindow()->getAppDir();
		QString const logpath = appdir + "/" + m_path.join("/");
		return logpath;
	}

	void LogWidget::loadConfig (QString const & preset_dir)
	{
		QString const logpath = preset_dir + "/" + g_presetLogTag;
		m_config.clear();
		bool const loaded = logs::loadConfig(m_config, logpath + "/" + m_config.m_tag);
		if (!loaded)
			m_connection->defaultConfigFor(m_config);
		
		loadAuxConfigs();
	}
	void LogWidget::loadAuxConfigs ()
	{
		QString const logpath = getCurrentPresetPath();
		m_config.m_find_config.clear();
		loadConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
		filterMgr()->loadConfig(logpath);
	}
	void LogWidget::saveAuxConfigs ()
	{
		QString const logpath = getCurrentPresetPath();
		saveConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
		filterMgr()->saveConfig(logpath);
	}
	void LogWidget::saveFindConfig ()
	{
		QString const logpath = getCurrentPresetPath();
		saveConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
	}

	void LogWidget::normalizeConfig (logs::LogConfig & normalized)
	{
		QMap<int, int> perms;
		int const n = horizontalHeader()->count();
		for (int i = 0; i < n; ++i)
		{
			int const currentVisualIndex = horizontalHeader()->visualIndex(i);
			if (currentVisualIndex != i)
			{
				perms.insert(i, currentVisualIndex);
			}
		}

		normalized.m_columns_setup.resize(n);
		normalized.m_columns_sizes.resize(n);
		normalized.m_columns_align.resize(n);
		normalized.m_columns_elide.resize(n);

		QMapIterator<int, int> iter(perms);
		while (iter.hasNext())
		{
			iter.next();
			int const logical = iter.key();
			int const visual = iter.value();

			normalized.m_columns_setup[visual] = m_config.m_columns_setup[logical];
			normalized.m_columns_sizes[visual] = m_config.m_columns_sizes[logical];
			normalized.m_columns_align[visual] = m_config.m_columns_align[logical];
			normalized.m_columns_elide[visual] = m_config.m_columns_elide[logical];
		}
	}

	void LogWidget::saveConfig (QString const & path)
	{
		QString const logpath = path + "/" + g_presetLogTag;
		mkDir(logpath);

		logs::LogConfig tmp = m_config;
		normalizeConfig(tmp);
		logs::saveConfig(tmp, logpath + "/" + m_config.m_tag);
		saveAuxConfigs();

        //currentIndex  = horizontalHeader()->visualIndex(session.findColumn4Tag(iter.key()));
		// isSectionHidden
	}

	void LogWidget::onSaveButton ()
	{
		/*m_config.m_hsize.clear();
		m_config.m_hsize.resize(m_modelView->columnCount());
		for (int i = 0, ie = m_modelView->columnCount(); i < ie; ++i)
			m_config.m_hsize[i] = horizontalHeader()->sectionSize(i);*/
		//saveConfig();
		//m_pers_filter.saveConfig(
	}
	void LogWidget::onResetButton () { setConfigValuesToUI(m_config); }
	void LogWidget::onDefaultButton ()
	{
		LogConfig defaults;
		//defaults.partialLoadFrom(m_config);
		setConfigValuesToUI(defaults);
	}

	void LogWidget::onClearAllDataButton ()
	{
	}

	/*void BaseLog::performTimeSynchronization (int sync_group, unsigned long long time, void * source)
	{
		qDebug("%s syncgrp=%i time=%i", __FUNCTION__, sync_group, time);
	}

	void BaseLog::performFrameSynchronization (int sync_group, unsigned long long frame, void * source)
	{
		qDebug("%s syncgrp=%i frame=%i", __FUNCTION__, sync_group, frame);
	}*/

void LogWidget::onNextToView ()
{
	nextToView();
}

void LogWidget::turnOffAutoScroll ()
{
	//ui->autoScrollCheckBox->setCheckState(Qt::Unchecked);
}

void LogWidget::onAutoScrollHotkey ()
{
	/*if (ui->autoScrollCheckBox->checkState() == Qt::Checked)
		turnOffAutoScroll();
	else
		ui->autoScrollCheckBox->setCheckState(Qt::Checked);*/
}

void LogWidget::onFilterEnabledChanged ()
{
	applyConfig(m_config);
	//setupUi
	//applyConfig();
}

void LogWidget::onTableFontToolButton ()
{
    /*bool ok = false;
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
    }*/
}


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

void LogWidget::onDumpFilters ()
{
	/*QDialog dialog(this);
	dialog.setWindowFlags(Qt::Sheet);
	m_help->setupUi(&dialog);
	m_help->helpTextEdit->clear();

	QString text(tr("Dumping current filters:\n"));
	QString session_string;

	if (Connection * conn = m_server->findCurrentConnection())
	{
		SessionState const & ss = conn->sessionState();
		QString ff;
		ss.m_file_filters.dump_filter(ff);

		QString cols;
		for (int i = 0, ie = ss.m_colorized_texts.size(); i < ie; ++i)
		{
			ColorizedText const & x = ss.m_colorized_texts.at(i);
			cols += QString("%1 %2 %3\n")
						.arg(x.m_regex_str)
						.arg(x.m_qcolor.name())
						.arg(x.m_is_enabled ? " on" : "off");
		}
		QString regs;
		for (int i = 0, ie = ss.m_filtered_regexps.size(); i < ie; ++i)
		{
			FilteredRegex const & x = ss.m_filtered_regexps.at(i);
			regs += QString("%1 %2 %3\n")
						.arg(x.m_regex_str)
						.arg(x.m_state ? "incl" : "excl")
						.arg(x.m_is_enabled ? " on" : "off");
		}
		QString lvls;
		for (int i = 0, ie = ss.m_lvl_filters.size(); i < ie; ++i)
		{
			FilteredLevel const & x = ss.m_lvl_filters.at(i);
			lvls += QString("%1 %2 %3\n")
						.arg(x.m_level_str)
						.arg(x.m_is_enabled ? " on" : "off")
						.arg(x.m_state ? "incl" : "excl");
		}
		QString ctxs;
		for (int i = 0, ie = ss.m_ctx_filters.size(); i < ie; ++i)
		{
			FilteredContext const & x = ss.m_ctx_filters.at(i);
			ctxs += QString("%1 %2 %3\n")
						.arg(x.m_ctx_str)
						.arg(x.m_state)
						.arg(x.m_is_enabled ? " on" : "off");
		}

		session_string = QString("Files:\n%1\n\nColors:\n%2\n\nRegExps:\n%3\n\nLevels:\n%4\n\nContexts:\n%5\n\n")
				.arg(ff)
				.arg(cols)
				.arg(regs)
				.arg(lvls)
				.arg(ctxs);
	}

	m_help->helpTextEdit->setPlainText(text + session_string);
	m_help->helpTextEdit->setReadOnly(true);
	dialog.exec();*/
}

DecodedCommand const * LogWidget::getDecodedCommand (QModelIndex const & row_index)
{
	return getDecodedCommand(row_index.row());
}

DecodedCommand const * LogWidget::getDecodedCommand (int row)
{
	if (row >= 0 && row < m_src_model->dcmds().size())
		return &m_src_model->dcmds()[row];
	return 0;
}

void LogWidget::commitCommands (E_ReceiveMode mode)
{
	for (int i = 0, ie = m_queue.size(); i < ie; ++i)
	{
		DecodedCommand & cmd = m_queue[i];
		m_src_model->handleCommand(cmd, mode);
		appendToFilters(cmd);
	}
	if (m_queue.size())
	{
		m_src_model->commitCommands(mode);
		m_queue.clear();
	}
}

void LogWidget::handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	if (mode == e_RecvSync)
		m_src_model->handleCommand(cmd, mode);
	else
		m_queue.append(cmd);

/*	if (cmd.hdr.cmd == tlv::cmd_scope_entry || (cmd.hdr.cmd == tlv::cmd_scope_exit))
	{
		if (m_config.m_scopes)
		{
			LogTableModel * m = static_cast<LogTableModel *>(m_proxy_model ? m_proxy_model->sourceModel() : model());
			//dp.widget().model()->appendCommand(m_proxy_model, cmd);
			m->appendCommand(m_proxy_model, cmd);
		}
	}
	else if (cmd.hdr.cmd == tlv::cmd_log)
	{
		m_src_model->handleCommand(cmd);
		//m_src_model->appendCommand(m_proxy_model, cmd);
	}*/
}

LogTableModel * LogWidget::cloneToNewModel (FindConfig const & fc)
{
	if (model() == m_src_model)
	{
		return m_src_model->cloneToNewModel(fc);
	}
	else if (model() == m_proxy_model)
	{
		Q_ASSERT(m_src_model);
		LogTableModel * new_model = new LogTableModel(this, *this);
		for (size_t r = 0, re = m_src_model->dcmds().size(); r < re; ++r)
		{
			DecodedCommand const & dcmd = m_src_model->dcmds()[r];
			if (m_proxy_model->filterAcceptsRow(r, QModelIndex()))
			{
				bool row_match = false;
				for (size_t i = 0, ie = dcmd.m_tvs.size(); i < ie; ++i)
				{
					QString const & val = dcmd.m_tvs[i].m_val;

					if (matchToFindConfig(val, fc))
					{
						row_match = true;
						break;
					}
				}

				if (row_match)
				{
					new_model->handleCommand(dcmd, e_RecvBatched);
				}
			}

		}
		new_model->commitCommands(e_RecvSync);
		return new_model;
	}
}



bool LogWidget::handleAction (Action * a, E_ActionHandleType sync)
{
	switch (a->type())
	{
		case e_Visibility:
		{
			Q_ASSERT(m_args.size() > 0);
			bool const on = a->m_args.at(0).toBool();
			setVisible(on);
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
		default:
			return false;
	}
	return false;
}

/*void LogWidget::applyConfig ()
{
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

int LogWidget::findColumn4Tag (tlv::tag_t tag)
{
	QMap<tlv::tag_t, int>::const_iterator it = m_tags2columns.find(tag);
	if (it != m_tags2columns.end())
		return it.value();

	char const * name = tlv::get_tag_name(tag);
	QString const qname = QString(name);
	for (int i = 0, ie = m_config.m_columns_setup.size(); i < ie; ++i)
		if (m_config.m_columns_setup[i] == qname)
		{
			m_tags2columns.insert(tag, i);
			return i;
		}
	return -1;
}

int LogWidget::appendColumn (tlv::tag_t tag)
{
	TagDesc const & desc = m_tagconfig.findOrCreateTag(tag);

	m_config.m_columns_setup.push_back(desc.m_tag_str);
	m_config.m_columns_align.push_back(desc.m_align_str);
	m_config.m_columns_elide.push_back(desc.m_elide_str);
	m_config.m_columns_sizes.push_back(desc.m_size);

	//qDebug("inserting column and size. tmpl_sz=%u curr_sz=%u sizes_sz=%u", m_columns_setup_template->size(), m_columns_setup_current->size(), m_columns_sizes->size());

	int const column_index = m_config.m_columns_setup.size() - 1;
	m_tags2columns.insert(tag, column_index);

	return column_index;
}

QString LogWidget::onCopyToClipboard ()
{
	QAbstractItemModel * m = model();
	QItemSelectionModel * selection = selectionModel();
	if (!selection)
		return QString();
	QModelIndexList indexes = selection->selectedIndexes();

	if (indexes.size() < 1)
		return QString();

	std::sort(indexes.begin(), indexes.end());

	QString selected_text;
	selected_text.reserve(4096);
	for (int i = 0; i < indexes.size(); ++i)
	{
		QModelIndex const current = indexes.at(i);
		selected_text.append(m->data(current).toString());
		
		if (i + 1 < indexes.size() && current.row() != indexes.at(i + 1).row())
			selected_text.append('\n');	// switching rows
		else
			selected_text.append('\t');
	}
	return selected_text;
}

bool LogWidget::isModelProxy () const
{
	if (0 == model())
		return false;
	return model() == m_proxy_model;
}

void LogWidget::onFilterChanged ()
{
	onInvalidateFilter();
}

void LogWidget::findTableIndexInFilters (QModelIndex const & src_idx, bool scroll_to_item, bool expand)
{
/*	{
		QString const file = findString4Tag(tlv::tag_file, src_idx);
		QString const line = findString4Tag(tlv::tag_line, src_idx);
		QString const combined = file + "/" + line;
		qDebug("findTableIndexInFilters %s in tree", combined.toStdString().c_str());
		m_file_model->selectItem(m_main_window->getWidgetFile(), combined, scroll_to_item);
		if (expand)
			m_file_model->expandItem(m_main_window->getWidgetFile(), combined);
	}
	{
		QString tid = findString4Tag(tlv::tag_tid, src_idx);
		QModelIndexList indexList = m_tid_model->match(m_tid_model->index(0, 0), Qt::DisplayRole, tid);
		if (!indexList.empty())
		{
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetTID()->setCurrentIndex(selectedIndex);
		}
	}
	{
		QString lvl = findString4Tag(tlv::tag_lvl, src_idx);
		QModelIndexList indexList = m_lvl_model->match(m_lvl_model->index(0, 0), Qt::DisplayRole, lvl);
		if (!indexList.empty())
		{
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetLvl()->setCurrentIndex(selectedIndex);
		}
	}
	{
		QString ctx = findString4Tag(tlv::tag_ctx, src_idx);
		QModelIndexList indexList = m_ctx_model->match(m_ctx_model->index(0, 0), Qt::DisplayRole, ctx);
		if (!indexList.empty())
		{
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetCtx()->setCurrentIndex(selectedIndex);
		}
	}*/
}

void LogWidget::onTableClicked (QModelIndex const & row_index)
{
/*	m_last_clicked = row_index;
	if (isModelProxy())
		m_last_clicked = m_proxy_model->mapToSource(row_index);

	m_last_search_row = m_last_clicked.row(); // set search from this line
	m_last_search_col = m_last_clicked.column();

	bool const scroll_to_item = false;
	bool const expand = false;
	findTableIndexInFilters(m_last_clicked, scroll_to_item, expand);*/
}

void LogWidget::onTableDoubleClicked (QModelIndex const & row_index)
{
/*	if (m_proxy_model)
	{
		QModelIndex const curr = m_proxy_model->mapToSource(row_index);
		LogTableModel * model = static_cast<LogTableModel *>(m_proxy_model ? m_proxy_model->sourceModel() : model());
		qDebug("2c curr: (%i,col) -> (%i,col)", row_index.row(), curr.row());

		int row_bgn = curr.row();
		int row_end = curr.row();
		int layer = model->layers()[row_bgn];

		if (model->rowTypes()[row_bgn] == tlv::cmd_scope_exit)
		{
			layer += 1;
			// test na range
			--row_bgn;
		}

		QString tid = findString4Tag(tlv::tag_tid, curr);
		QString file = findString4Tag(tlv::tag_file, curr);
		QString line = findString4Tag(tlv::tag_line, curr);
		int from = row_bgn;

		if (model->rowTypes()[from] != tlv::cmd_scope_entry)
		{
			while (row_bgn > 0)
			{
				QModelIndex const curr_idx = model->index(row_bgn, row_index.column(), QModelIndex());
				QString curr_tid = findString4Tag(tlv::tag_tid, curr_idx);
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
		if (model->rowTypes()[to] != tlv::cmd_scope_exit)
		{
			while (row_end < static_cast<int>(model->layers().size()))
			{
				QModelIndex const curr_idx = model->index(row_end, row_index.column(), QModelIndex());
				QString next_tid = findString4Tag(tlv::tag_tid, curr_idx);
				if (next_tid == tid)
				{
					if (model->layers()[row_end] >= layer)
						to = row_end;
					else if ((model->layers()[row_end] == layer - 1) && (model->rowTypes()[row_end] == tlv::cmd_scope_exit))
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

void LogWidget::onApplyColumnSetup ()
{
/*	qDebug("%s", __FUNCTION__);
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
*/
	//static_cast<TableView *>(m_table_view_widget)->setColumnOrder(order, m_session_state);
}

void LogWidget::onExcludeFileLine ()
{
	QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		onExcludeFileLine(current);
		onInvalidateFilter();
	}
}

void LogWidget::onToggleRefFromRow ()
{
	/*QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		qDebug("Toggle Ref from row=%i", current.row());
		m_session_state.setTimeRefFromRow(current.row());

		//LogTableModel * model = static_cast<LogTableModel *>(m_proxy_model ? m_proxy_model->sourceModel() : model());
		QString const & strtime = findString4Tag(tlv::tag_time, current);
		m_session_state.setTimeRefValue(strtime.toULongLong());
		onInvalidateFilter();
	}*/
}

void LogWidget::onColorTagRow (int)
{
	/*QModelIndex current = currentIndex();
	if (isModelProxy())
	{
		current = m_proxy_model->mapToSource(current);
	}

	int const row = current.row(); // set search from this line
	if (current.isValid())
	{
		qDebug("Color tag on row=%i", current.row());
		m_session_state.addColorTagRow(current.row());
		onInvalidateFilter();
	}*/
}

void LogWidget::onClearCurrentView ()
{
	/*LogTableModel * model = static_cast<LogTableModel *>(m_proxy_model ? m_proxy_model->sourceModel() : model());
	m_session_state.excludeContentToRow(model->rowCount());
	onInvalidateFilter();*/
}

void LogWidget::onHidePrevFromRow ()
{
	/*QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		qDebug("Hide prev from row=%i", current.row());
		m_session_state.excludeContentToRow(current.row());
		onInvalidateFilter();
	}*/
}

void LogWidget::onUnhidePrevFromRow ()
{
	//m_session_state.excludeContentToRow(0);
	//onInvalidateFilter();
}

/*
void LogWidget::onShowContextMenu (QPoint const & pos)
{
    QPoint globalPos = mapToGlobal(pos);
    QAction * selectedItem = m_ctx_menu.exec(globalPos); // @TODO: rather async

	//poas = ui->tableView->viewport()->mapFromGlobal(e->globalPos());
	QModelIndex const idx = indexAt(pos);
	qDebug("left click at r=%2i,c=%2i", idx.row(), idx.column());

	onTableClicked(idx);

    if (selectedItem == m_actions[e_action_HidePrev])
    {
		onHidePrevFromRow();
    }
    else if (selectedItem == m_actions[e_action_ToggleRef])
    {
		onToggleRefFromRow();
    }
    else if (selectedItem == m_actions[e_action_ExcludeFileLine])
	{
		onExcludeFileLine(m_last_clicked);
	}
    else if (selectedItem == m_actions[e_action_Find])
	{
		onFindFileLine(m_last_clicked);
	}
    else if (selectedItem == m_actions[e_action_Copy])
	{
		QString const & selection = onCopyToClipboard();
		qApp->clipboard()->setText(selection);
	}
    else if (selectedItem == m_actions[e_action_ColorTag])
	{
		onColorTagRow(m_last_clicked.row());
	}
    else if (selectedItem == m_actions[e_action_Setup])
	{
	}
    else
    { }
}
*/

void LogWidget::onSectionMoved (int logical, int old_visual, int new_visual)
{
	qDebug("log: section moved logical=%i old_visual=%i new_visual=%i", logical, old_visual, new_visual);
}

void LogWidget::onSectionResized (int logical, int old_size, int new_size)
{
	qDebug("log: section resized logical=%i old_sz=%i new_sz=%i", logical, old_size, new_size);
	/*if (idx < m_config.m_columns_setup.size())
	{
		m_config.m_columns_sizes[idx] = new_size;
	}*/

	//@TODO: proxy !!!!!!!!!!!!!!!!!!
	int const idx = !isModelProxy() ? logical : m_proxy_model->colToSource(logical);
	//qDebug("table: on rsz hdr[%i -> src=%02i ]  %i->%i\t\t%s", c, idx, old_size, new_size, m_config.m_hhdr.at(idx).toStdString().c_str());
	if (idx < 0) return;
	int const curr_sz = m_config.m_columns_sizes.size();
	if (idx < curr_sz)
	{
		//qDebug("%s this=0x%08x hsize[%i]=%i", __FUNCTION__, this, idx, new_size);
	}
	else
	{
		m_config.m_columns_sizes.resize(idx + 1);
		for (int i = curr_sz; i < idx + 1; ++i)
			m_config.m_columns_sizes[i] = 32;
	}
	m_config.m_columns_sizes[idx] = new_size;
}

void LogWidget::exportStorageToCSV (QString const & filename)
{
	// " --> ""
	QRegExp regex("\"");
	QString to_string("\"\"");
	QFile csv(filename);
	csv.open(QIODevice::WriteOnly);
	QTextStream str(&csv);

	for (int c = 0, ce = m_config.m_columns_setup.size(); c < ce; ++c)
	{
		str << "\"" << m_config.m_columns_setup.at(c) << "\"";
		if (c < ce - 1)
			str << ",\t";
	}
	str << "\n";

	for (int r = 0, re = model()->rowCount(); r < re; ++r)
	{
		for (int c = 0, ce = model()->columnCount(); c < ce; ++c)
		{
			QModelIndex current = model()->index(r, c, QModelIndex());
			// csv nedumpovat pres proxy
			QString txt = model()->data(current).toString();
			QString const quoted_txt = txt.replace(regex, to_string);
			str << "\"" << quoted_txt << "\"";
			if (c < ce - 1)
				str << ",\t";
		}
		str << "\n";
	}
	csv.close();
}


}


