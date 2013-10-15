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
#include <QInputDialog>
#include <QFontDialog>


namespace logs {

	LogWidget::LogWidget (Connection * connection, QWidget * wparent, LogConfig & cfg, QString const & fname, QStringList const & path)
		: TableView(wparent), ActionAble(path)
		, m_config(cfg)
		, m_config2(cfg)
		, m_config_ui(cfg, this)
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
		, m_src_model(0)
		, m_selection(0)
		, m_src_selection(0)
		, m_ctx_menu()
		, m_actions()
		, m_last_clicked()
		, m_csv_separator()
		, m_file_csv_stream(0)
		//, m_file_tlv_stream(0)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		//
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

		setModel(model);

		m_src_model = model;

		//m_proxy_selection = new QItemSelectionModel(m_proxy_model);
		setupLogSelectionProxy();

		m_proxy_model = new FilterProxyModel(this, *this);
		m_proxy_model->setSourceModel(m_src_model);
		setupFilteringProxy(filterMgr()->enabled() ? Qt::Checked : Qt::Unchecked);
	}

	void LogWidget::setupLogSelectionProxy ()
	{
		m_src_selection = new QItemSelectionModel(m_src_model);
		setSelectionModel(m_src_selection);
		m_selection = new LogSelectionProxyModel(m_src_model, m_src_selection);
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
		setupFilteringProxy(filterMgr()->enabled() ? Qt::Checked : Qt::Unchecked);

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

	void LogWidget::loadConfig (QString const & preset_dir)
	{
		QString const logpath = preset_dir + "/" + g_presetLogTag;
		m_config.clear();
		bool const loaded = logs::loadConfig(m_config, logpath + "/" + m_config.m_tag);
		if (!loaded)
			m_connection->defaultConfigFor(m_config);

		filterMgr()->loadConfig(logpath);
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
		filterMgr()->saveConfig(logpath);

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


void LogWidget::onEditFind ()
{
	/*bool ok;
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
	}*/
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
	/*else
	{
		size_t const tag_idx = tlv::tag_for_name(qcolumn.toStdString().c_str());
		if (tag_idx != tlv::tag_invalid)
		{
			conn->findText(text, tag_idx);
		}
	}*/
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

void LogWidget::onQFilterLineEditFinished ()
{
	/*if (ui->qFilterLineEdit->text().size() == 0)
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

	recompileStrings();*/
}

void LogWidget::appendToSearchHistory (QString const & str)
{
/*	if (str.length() == 0)
		return;
	m_config.m_search_history.insert(str);
	m_config.saveSearchHistory();
	updateSearchHistory();
	ui->qSearchComboBox->setCurrentIndex(ui->qSearchComboBox->findText(str));*/
}

void LogWidget::updateSearchHistory ()
{
/*	ui->qSearchComboBox->clear();
	for (size_t i = 0, ie = m_config.m_search_history.size(); i < ie; ++i)
		ui->qSearchComboBox->addItem(m_config.m_search_history[i]);*/
}

void LogWidget::onEditFindNext ()
{
	//findNext(ui->qSearchComboBox->currentText());
}

void LogWidget::onEditFindPrev ()
{
	//findPrev(ui->qSearchComboBox->currentText());
}

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
	m_src_model->commitCommands(mode);
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
			if (a->m_args.size() == 5)
			{
				QString const str  = a->m_args.at(0).toString();	// text
				bool const ww = a->m_args.at(1).toBool();			// m_whole_word;
				bool const Aa = a->m_args.at(2).toBool();			// m_case_sensitive
				bool const regexp = a->m_args.at(3).toBool();		// m_regexp
				QString const to_w  = a->m_args.at(4).toString();	// m_to_widget

				m_config.m_find_config.m_whole_word = ww;
				m_config.m_find_config.m_case_sensitive = Aa;
				m_config.m_find_config.m_regexp = regexp;
				m_config.m_find_config.m_to_widget = to_w;
				m_config.m_find_config.m_str = str;
				m_config.m_find_config.m_history.insert(str);
				// m_config.save
			}
			return true;
		}
		default:
			return false;
	}
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



#if 0

#include "ui_settings.h"
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QStandardItem>
#include "types.h"
#include "utils.h"
#include "utils_qstandarditem.h"
#include "mainwindow.h"
#include "connection.h"
#include "movablelistmodel.h"
#include <tlv_parser/tlv_parser.h>



void MainWindow::syncSettingsViews (QListView const * const invoker, QModelIndex const idx)
{
	QListView * const views[] = { ui_settings->listViewColumnSetup, ui_settings->listViewColumnSizes, ui_settings->listViewColumnAlign, ui_settings->listViewColumnElide };

	for (size_t i = 0; i < sizeof(views) / sizeof(*views); ++i)
	{
		if (views[i] != invoker)
		{
			views[i]->selectionModel()->clearSelection();
			QModelIndex const other_idx = views[i]->model()->index(idx.row(), idx.column(), QModelIndex());
			views[i]->selectionModel()->select(other_idx, QItemSelectionModel::Select);
		}
	}
}

void MainWindow::onClickedAtSettingColumnSetup (QModelIndex const idx)
{
	syncSettingsViews(ui_settings->listViewColumnSetup, idx);

	QStandardItem * const item = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSetup->model())->itemFromIndex(idx);
	Qt::CheckState const curr = item->checkState();

	item->setCheckState(curr == Qt::Checked ? Qt::Unchecked : Qt::Checked);

	QModelIndex const size_idx = ui_settings->listViewColumnSizes->model()->index(idx.row(), idx.column(), QModelIndex());
	if (curr == Qt::Checked)
	{
		//ui_settings->listViewColumnSizes->model()->setData(size_idx, QString("0"));
	}
	else
	{
		// this does not work at all
		/*int app_idx = 0;
		Connection * conn = m_server->findCurrentConnection();
		if (conn)
			app_idx = conn->sessionState().m_app_idx;

		int size_val = 64;
		if (app_idx < m_columns_sizes.size())
		{
			if (size_idx.row() < m_columns_sizes[app_idx].size())
			{
				size_val = m_columns_sizes[app_idx].at(size_idx.row());
				ui_settings->listViewColumnSizes->model()->setData(size_idx, tr("%1").arg(size_val));
			}
		}*/
	}
}
void MainWindow::onClickedAtSettingColumnSizes (QModelIndex const idx)
{
	syncSettingsViews(ui_settings->listViewColumnSizes, idx);
}
void MainWindow::onClickedAtSettingColumnAlign (QModelIndex const idx)
{
	QString const txt = ui_settings->listViewColumnAlign->model()->data(idx).toString();
	E_Align const curr = stringToAlign(txt.toStdString().c_str()[0]);
	size_t i = (curr + 1) % e_max_align_enum_value;
	E_Align const act = static_cast<E_Align>(i);
	ui_settings->listViewColumnAlign->model()->setData(idx, QString(alignToString(act)));

	syncSettingsViews(ui_settings->listViewColumnAlign, idx);
}
void MainWindow::onClickedAtSettingColumnElide (QModelIndex const idx)
{
	QString const txt = ui_settings->listViewColumnElide->model()->data(idx).toString();
	E_Elide const curr = stringToElide(txt.toStdString().c_str()[0]);
	size_t i = (curr + 1) % e_max_elide_enum_value;
	E_Elide const act = static_cast<E_Elide>(i);
	ui_settings->listViewColumnElide->model()->setData(idx, QString(elideToString(act)));

	syncSettingsViews(ui_settings->listViewColumnElide, idx);
}

void MainWindow::onSettingsAppSelectedTLV (int const idx, bool const first_time)
{
	qDebug("settings, clicked idx=%i", idx);
	clearListView(ui_settings->listViewColumnSetup);
	clearListView(ui_settings->listViewColumnSizes);
	clearListView(ui_settings->listViewColumnAlign);
	clearListView(ui_settings->listViewColumnElide);

	ui_settings->listViewColumnSetup->reset();
	ui_settings->listViewColumnSizes->reset();
	ui_settings->listViewColumnAlign->reset();
	ui_settings->listViewColumnElide->reset();

	QStandardItem * cs_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSetup->model())->invisibleRootItem();
	QStandardItem * csz_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSizes->model())->invisibleRootItem();
	QStandardItem * cal_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnAlign->model())->invisibleRootItem();
	QStandardItem * cel_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnElide->model())->invisibleRootItem();
	if (idx >= 0 && idx < m_config.m_columns_setup.size())
		for (int i = 0, ie = m_config.m_columns_setup[idx].size(); i < ie; ++i)
		{
			cs_root->appendRow(addRow(m_config.m_columns_setup.at(idx).at(i), true));
			csz_root->appendRow(addUncheckableRow(tr("%1").arg(m_config.m_columns_sizes.at(idx).at(i))));
			cal_root->appendRow(addUncheckableRow(tr("%1").arg(m_config.m_columns_align.at(idx).at(i))));
			cel_root->appendRow(addUncheckableRow(tr("%1").arg(m_config.m_columns_elide.at(idx).at(i))));
		}

	//size_t const n = tlv::get_tag_count() - 1; // -1 is for the tag Bool
	size_t const n = tlv::tag_bool;

	size_t add_tag_count = 0;
	size_t * const add_tag_indices = static_cast<size_t * const>(alloca(sizeof(size_t) * n));
	for (size_t i = tlv::tag_time; i < n; ++i)
	{
		char const * name = tlv::get_tag_name(i);
		if (name)
		{
			if (findChildByText(cs_root, QString::fromLatin1(name)))
				continue;

			QList<QStandardItem *> row_items = addRow(QString::fromLatin1(name), first_time);
			cs_root->appendRow(row_items);
			add_tag_indices[add_tag_count++] = i;

			csz_root->appendRow(addUncheckableRow(QString("0")));
			cal_root->appendRow(addUncheckableRow(QString(aligns[0])));
			cel_root->appendRow(addUncheckableRow(QString(elides[0])));
		}
	}

	disconnect(ui_settings->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	disconnect(ui_settings->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	disconnect(ui_settings->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	disconnect(ui_settings->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));
	connect(ui_settings->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	connect(ui_settings->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	connect(ui_settings->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	connect(ui_settings->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));

	connect(ui_settings->macUserButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingPooftahButton()));
	connect(ui_settings->okButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingOkButton()));
	connect(ui_settings->okSaveButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingOkSaveButton()));
	connect(ui_settings->cancelButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingCancelButton()));
}


void MainWindow::onSettingsAppSelectedCSV (int const idx, int const columns, bool const first_time)
{
	qDebug("settings, clicked idx=%i", idx);
	clearListView(ui_settings->listViewColumnSetup);
	clearListView(ui_settings->listViewColumnSizes);
	clearListView(ui_settings->listViewColumnAlign);
	clearListView(ui_settings->listViewColumnElide);

	ui_settings->listViewColumnSetup->reset();
	ui_settings->listViewColumnSizes->reset();
	ui_settings->listViewColumnAlign->reset();
	ui_settings->listViewColumnElide->reset();

	QStandardItem * cs_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSetup->model())->invisibleRootItem();
	QStandardItem * csz_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSizes->model())->invisibleRootItem();
	QStandardItem * cal_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnAlign->model())->invisibleRootItem();
	QStandardItem * cel_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnElide->model())->invisibleRootItem();
	if (idx >= 0 && idx < m_config.m_columns_setup.size())
		for (int i = 0, ie = m_config.m_columns_setup[idx].size(); i < ie; ++i)
		{
			cs_root->appendRow(addRow(m_config.m_columns_setup.at(idx).at(i), true));
			csz_root->appendRow(addUncheckableRow(tr("%1").arg(m_config.m_columns_sizes.at(idx).at(i))));
			cal_root->appendRow(addUncheckableRow(tr("%1").arg(m_config.m_columns_align.at(idx).at(i))));
			cel_root->appendRow(addUncheckableRow(tr("%1").arg(m_config.m_columns_elide.at(idx).at(i))));
		}

	/*
	//size_t const n = tlv::get_tag_count() - 1; // -1 is for the tag Bool
	size_t const n = tlv::tag_bool;

	size_t add_tag_count = 0;
	size_t * const add_tag_indices = static_cast<size_t * const>(alloca(sizeof(size_t) * n));
	for (size_t i = tlv::tag_time; i < n; ++i)
	{
		char const * name = tlv::get_tag_name(i);
		if (name)
		{
			if (findChildByText(cs_root, QString::fromAscii(name)))
				continue;

			QList<QStandardItem *> row_items = addRow(QString::fromAscii(name), first_time);
			cs_root->appendRow(row_items);
			add_tag_indices[add_tag_count++] = i;

			csz_root->appendRow(addUncheckableRow(QString("0")));
			cal_root->appendRow(addUncheckableRow(QString(aligns[0])));
			cel_root->appendRow(addUncheckableRow(QString(elides[0])));
		}
	}*/

	disconnect(ui_settings->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	disconnect(ui_settings->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	disconnect(ui_settings->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	disconnect(ui_settings->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));
	connect(ui_settings->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	connect(ui_settings->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	connect(ui_settings->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	connect(ui_settings->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));

	/*connect(ui_settings->macUserButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingPooftahButton()));
	connect(ui_settings->okButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingOkButton()));
	connect(ui_settings->okSaveButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingOkSaveButton()));
	connect(ui_settings->cancelButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingCancelButton()));*/

	connect(ui_settings->okButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingOkButton()));
	connect(ui_settings->okSaveButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingOkSaveButton()));
	connect(ui_settings->cancelButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingCancelButton()));
}

void MainWindow::onSetupAction ()
{
	// TODO: protocol from current tab
	onSetup(e_Proto_TLV, -1);
}


void MainWindow::setupSeparatorChar (QString const & c)
{
	ui_settings->separatorComboBox->addItem(c);
	ui_settings->separatorComboBox->setCurrentIndex(ui_settings->separatorComboBox->findText(c));
}

QString MainWindow::separatorChar () const
{
	return ui_settings->protocolComboBox->currentText();
}

void MainWindow::onSetup (E_SrcProtocol const proto, int curr_app_idx, bool first_time, bool mac_user)
{
	settingsToDefault();
	prepareSettingsWidgets(curr_app_idx, first_time);

	if (proto == e_Proto_TLV)
	{
		ui_settings->separatorComboBox->setEnabled(false);
		ui_settings->columnCountSpinBox->setEnabled(false);
		ui_settings->protocolComboBox->setCurrentIndex(ui_settings->protocolComboBox->findText("TLV"));
		onSettingsAppSelectedTLV(curr_app_idx, first_time);
	}
	else
	{
		ui_settings->protocolComboBox->setCurrentIndex(ui_settings->protocolComboBox->findText("CSV"));
		onSettingsAppSelectedCSV(curr_app_idx, first_time);
	}

	if (mac_user)
		onClickedAtSettingPooftahButton();

	m_settings_dialog->exec();

	clearSettingWidgets();
}

void MainWindow::onSetupCSVColumns (int curr_app_idx, int columns, bool first_time)
{
	prepareSettingsWidgets(curr_app_idx, first_time);
	ui_settings->separatorComboBox->setEnabled(false);
	ui_settings->columnCountSpinBox->setEnabled(true);
	ui_settings->columnCountSpinBox->setValue(columns);
	ui_settings->protocolComboBox->setCurrentIndex(ui_settings->protocolComboBox->findText("CSV"));

	// @TODO: fill widgets

	connect(ui_settings->separatorComboBox, SIGNAL(activated(int)), this, SLOT(onSeparatorSelected(int)));
	onSettingsAppSelectedCSV(curr_app_idx, columns, first_time);

	m_settings_dialog->exec();
	clearSettingWidgets();
}

void MainWindow::settingsToDefault ()
{
	ui_settings->separatorComboBox->setDuplicatesEnabled(false);
	ui_settings->protocolComboBox->setDuplicatesEnabled(false);
	ui_settings->protocolComboBox->addItem("CSV");
	ui_settings->protocolComboBox->addItem("TLV");
	ui_settings->columnCountSpinBox->setEnabled(false);

}

void MainWindow::settingsDisableButSeparator ()
{
	ui_settings->separatorComboBox->setEnabled(true);
	ui_settings->columnCountSpinBox->setEnabled(false);
}

void MainWindow::onSetupCSVSeparator (int curr_app_idx, bool first_time)
{
	settingsToDefault();
	prepareSettingsWidgets(curr_app_idx, first_time);
	settingsDisableButSeparator();

	ui_settings->protocolComboBox->setCurrentIndex(ui_settings->protocolComboBox->findText("CSV"));
	onSettingsAppSelectedCSV(curr_app_idx, first_time);

	m_settings_dialog->exec();
	clearSettingWidgets();
}

void MainWindow::prepareSettingsWidgets (int curr_app_idx, bool first_time)
{
	if (curr_app_idx == -1)
	{
		ui_settings->appNameComboBox->setEnabled(true);
		ui_settings->appNameComboBox->clear();
		for (int a = 0, ae = m_config.m_app_names.size(); a < ae; ++a)
			ui_settings->appNameComboBox->addItem(m_config.m_app_names.at(a));

		Connection * conn = m_server->findCurrentConnection();
		if (conn)
		{
			curr_app_idx = conn->sessionState().m_app_idx;
		}
	}
	else
	{
		ui_settings->appNameComboBox->clear();
		ui_settings->appNameComboBox->addItem(m_config.m_app_names.at(curr_app_idx));
		ui_settings->appNameComboBox->setEnabled(false);
	}

	connect(ui_settings->appNameComboBox, SIGNAL(activated(int)), this, SLOT(onSettingsAppSelected(int)));

	MyListModel * model = new MyListModel(this);
	ui_settings->listViewColumnSetup->setModel(model);
	//ui_settings->listViewColumnSetup->model()->setSupportedDragActions(Qt::MoveAction);
	ui_settings->listViewColumnSizes->setModel(new QStandardItemModel(this));
	ui_settings->listViewColumnAlign->setModel(new QStandardItemModel(this));
	ui_settings->listViewColumnElide->setModel(new QStandardItemModel(this));
	ui_settings->listViewColumnSetup->setDropIndicatorShown(true);
	ui_settings->listViewColumnSetup->setMovement(QListView::Snap);
	ui_settings->listViewColumnSetup->setDragDropMode(QAbstractItemView::InternalMove);
	model->addObserver(ui_settings->listViewColumnSizes->model());
	model->addObserver(ui_settings->listViewColumnAlign->model());
	model->addObserver(ui_settings->listViewColumnElide->model());
	ui_settings->listViewColumnSetup->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui_settings->listViewColumnAlign->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui_settings->listViewColumnElide->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::clearSettingWidgets()
{
	disconnect(ui_settings->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	disconnect(ui_settings->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	disconnect(ui_settings->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	disconnect(ui_settings->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));
	clearListView(ui_settings->listViewColumnSetup);
	clearListView(ui_settings->listViewColumnSizes);
	clearListView(ui_settings->listViewColumnAlign);
	clearListView(ui_settings->listViewColumnElide);
	ui_settings->listViewColumnSetup->reset();
	ui_settings->listViewColumnSizes->reset();
	ui_settings->listViewColumnAlign->reset();
	ui_settings->listViewColumnElide->reset();
}

void MainWindow::onClickedAtSettingPooftahButton ()
{
	for (size_t j = 0, je = ui_settings->listViewColumnAlign->model()->rowCount(); j < je; ++j)
	{
		QModelIndex const tag_idx = ui_settings->listViewColumnSetup->model()->index(j, 0, QModelIndex());
		QString const tag = ui_settings->listViewColumnSetup->model()->data(tag_idx).toString();

		QModelIndex const row_idx = ui_settings->listViewColumnAlign->model()->index(j, 0, QModelIndex());
		size_t const tag_val = tlv::tag_for_name(tag.toLatin1());
		ui_settings->listViewColumnAlign->model()->setData(row_idx, QString(alignToString(default_aligns[tag_val])));

		QModelIndex const erow_idx = ui_settings->listViewColumnElide->model()->index(j, 0, QModelIndex());
		ui_settings->listViewColumnElide->model()->setData(erow_idx, QString(elideToString(default_elides[tag_val])));

		QModelIndex const srow_idx = ui_settings->listViewColumnSizes->model()->index(j, 0, QModelIndex());
		ui_settings->listViewColumnSizes->model()->setData(srow_idx, tr("%1").arg(default_sizes[tag_val]));
	}
}

void MainWindow::onClickedAtSettingOkButton ()
{
	for (int app_idx = 0, app_idxe = m_config.m_app_names.size(); app_idx < app_idxe; ++app_idx)
	{
		qDebug("app=%s", m_config.m_app_names.at(app_idx).toStdString().c_str());
		m_config.m_columns_setup[app_idx].clear();
		m_config.m_columns_sizes[app_idx].clear();
		m_config.m_columns_align[app_idx].clear();
		m_config.m_columns_elide[app_idx].clear();

		for (size_t j = 0, je = ui_settings->listViewColumnSetup->model()->rowCount(); j < je; ++j)
		{
			QModelIndex const row_idx = ui_settings->listViewColumnSetup->model()->index(j, 0, QModelIndex());
			QStandardItem * const item = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSetup->model())->itemFromIndex(row_idx);
			if (item->checkState() == Qt::Checked)
			{
				QString const & d = ui_settings->listViewColumnSetup->model()->data(row_idx).toString();
				m_config.m_columns_setup[app_idx].append(d);
			}
		}
		for (size_t j = 0, je = ui_settings->listViewColumnSizes->model()->rowCount(); j < je; ++j)
		{
			QModelIndex const row_idx = ui_settings->listViewColumnSizes->model()->index(j, 0, QModelIndex());
			m_config.m_columns_sizes[app_idx].append(ui_settings->listViewColumnSizes->model()->data(row_idx).toString().toInt());
		}
		for (size_t j = 0, je = ui_settings->listViewColumnAlign->model()->rowCount(); j < je; ++j)
		{
			QModelIndex const row_idx = ui_settings->listViewColumnAlign->model()->index(j, 0, QModelIndex());
			m_config.m_columns_align[app_idx].append(ui_settings->listViewColumnAlign->model()->data(row_idx).toString());
		}
		for (size_t j = 0, je = ui_settings->listViewColumnElide->model()->rowCount(); j < je; ++j)
		{
			QModelIndex const row_idx = ui_settings->listViewColumnElide->model()->index(j, 0, QModelIndex());
			m_config.m_columns_elide[app_idx].append(ui_settings->listViewColumnElide->model()->data(row_idx).toString());
		}
	}

	m_settings_dialog->close();
	m_server->onApplyColumnSetup();
}

void MainWindow::onClickedAtSettingOkSaveButton ()
{
	onClickedAtSettingOkButton();
	storeState();
}

void MainWindow::onClickedAtSettingCancelButton ()
{
	m_settings_dialog->close();
}


	bool add (QString const & tlvname, int row, bool checked)
	{
		//insertRow(row, addRow(tlvname, checked));
		//beginInsertRows(QModelIndex(), row, row);
		//insertRow(m_data.count());
		//m_data.insert(row, tlvname);
		//endInsertRows();
		//emit layoutChanged();
		return true;
	}

	// @NOTE: hmm, that not work. me not know why. this no meat bone crush it!
	/*QVariant data (QModelIndex const & index, int role = Qt::DisplayRole) const {
		if (!index.isValid()) return QVariant();
		return QVariant(m_data[index.row()]);
	}
	int rowCount (QModelIndex const & parent) const {
		if (parent.isValid()) return 0;
		else return m_data.size();
	}
	int columnCount (QModelIndex const & parent) const { return 1; }*/

			//beginInsertRows(QModelIndex(), endRow, endRow);
			//m_data.insert(endRow, tlvname);
			//endInsertRows();

#endif



