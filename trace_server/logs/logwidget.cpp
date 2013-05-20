#include "logwidget.h"
//#include <QScrollBar>
//#include <QSplitter>
#include "../connection.h"
#include "../utils.h"
#include "../utils_qstandarditem.h"
//#include "delegates.h"
#include "../syncwidgets.h"


namespace logs {

	LogWidget::LogWidget (Connection * connection, QWidget * wparent, LogConfig & cfg, QString const & fname)
		: TableView(wparent)
		, m_config(cfg)
		, m_config_ui(cfg, this)
		, m_fname(fname)
		, m_connection(connection)
	, m_column_setup_done(false)
	, m_last_search_row(0)
	, m_last_search_col(0)
	, m_table_view_widget(0)
	, m_file_model(0)
	, m_file_proxy(0)
	, m_ctx_model(0)
	, m_func_model(0)
	, m_tid_model(0)
	, m_color_regex_model(0)
	, m_regex_model(0)
	, m_lvl_model(0)
	, m_string_model(0)
	, m_table_view_proxy(0)
	, m_table_view_src(0)
	, m_last_clicked()
	, m_file_csv_stream(0)
	//, m_file_tlv_stream(0)


	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		setConfigValuesToUI(m_config);
		onApplyButton();
		//setUpdatesEnabled(true);
		//horizontalHeader()->setSectionsMovable(true);
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
	LogTableModel * model = new LogTableModel(this, connection);
	QObject::disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), verticalHeader(), SLOT(sectionsInserted(QModelIndex,int,int)));
    verticalHeader()->setFont(config.m_font);
	verticalHeader()->setDefaultSectionSize(config.m_row_width);
	verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance
	setModel(model);
	QObject::connect(horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));

	m_table_view_src = model;
	m_table_view_widget = tableView;
	setupThreadColors(connection->getMainWindow()->getThreadColors());


	QStyle const * const style = QApplication::style();
	connect(m_config_ui->ui()->gotoNextButton, SIGNAL(clicked()), this, SLOT(onNextToView()));
	m_config_ui->ui()->gotoNextButton->setIcon(style->standardIcon(QStyle::SP_ArrowDown));
	//connect(ui->autoScrollCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onAutoScrollStateChanged(int)));
	//connect(ui->inViewCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onInViewStateChanged(int)));
	//
	//
	connect(ui->filterFileCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onFilterFile(int)));

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

	m_data_model = new TreeModel(this, &m_session_state.m_data_filters);
	
	m_delegates.get<e_delegate_Level>() = new LevelDelegate(m_session_state, this);
	m_delegates.get<e_delegate_Ctx>() = new CtxDelegate(m_session_state, this);
	m_delegates.get<e_delegate_String>() = new StringDelegate(m_session_state, this);
	m_delegates.get<e_delegate_Regex>() = new RegexDelegate(m_session_state, this);

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

	if (m_table_view_proxy)
	{
		m_table_view_proxy->setSourceModel(0);
		delete m_table_view_proxy;
		m_table_view_proxy = 0;
	}

	m_table_view_widget->setModel(0);
	delete m_table_view_src;
	m_table_view_src = 0;

	m_table_view_widget = 0;*/

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

	void LogWidget::applyConfig (LogConfig & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsLog * ui = m_config_ui.ui();

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
		setUIValuesToConfig(m_config);
		applyConfig(m_config);
	}

	void LogWidget::onSaveButton ()
	{
		/*m_config.m_hsize.clear();
		m_config.m_hsize.resize(m_modelView->columnCount());
		for (int i = 0, ie = m_modelView->columnCount(); i < ie; ++i)
			m_config.m_hsize[i] = horizontalHeader()->sectionSize(i);*/
		m_connection->saveConfigForLog(m_config, m_config.m_tag);

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

void MainWindow::onNextToView ()
{
	if (!getTabTrace()->currentWidget()) return;
	if (Connection * conn = m_server->findCurrentConnection())
		conn->nextToView();
}

void MainWindow::turnOffAutoScroll ()
{
	ui->autoScrollCheckBox->setCheckState(Qt::Unchecked);
}

void MainWindow::onAutoScrollHotkey ()
{
	if (ui->autoScrollCheckBox->checkState() == Qt::Checked)
		turnOffAutoScroll();
	else
		ui->autoScrollCheckBox->setCheckState(Qt::Checked);
}
}
void Server::onFilterFile (int state)
{
	if (Connection * conn = findCurrentConnection())
		conn->setFilterFile(state);
}

void LogWidget::onTableFontToolButton ()
{
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
		if (conn)
			conn->getTableViewWidget()->verticalHeader()->setFont(f);
    }
}


void MainWindow::onEditFind ()
{
	int const tab_idx = getTabTrace()->currentIndex();
	if (tab_idx < 0)
		return;

	bool ok;
	QString search = QInputDialog::getText(this, tr("Find"), tr("Text:"), QLineEdit::Normal, m_config.m_last_search, &ok);
	if (ok)
	{
		m_config.m_last_search = search;
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

void MainWindow::onQSearch (QString const & text)
{
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	appendToSearchHistory(text);
	//QString qcolumn = ui->qSearchColumnComboBox->currentText();
	//qDebug("onQSearch: col=%s text=%s", qcolumn.toStdString().c_str(), text.toStdString().c_str());
	//bool const search_all = (qcolumn == ".*");
	bool const search_all = true;
	if (search_all)
	{
		conn->findText(text);
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

void MainWindow::onQSearchEditingFinished ()
{
	if (!getTabTrace()->currentWidget()) return;

	QString const text = ui->qSearchComboBox->currentText();
	onQSearch(text);
}

void MainWindow::setLastSearchIntoCombobox (QString const & txt)
{
	ui->qSearchComboBox->addItem(txt);
	ui->qSearchComboBox->setCurrentIndex(ui->qSearchComboBox->findText(txt));
}

void MainWindow::onFindAllButton ()
{
	if (!getTabTrace()->currentWidget()) return;

	QString const text = ui->qSearchComboBox->currentText();
	if (Connection * conn = m_server->findCurrentConnection())
	conn->findAllTexts(text);
}

void MainWindow::onQFilterLineEditFinished ()
{
	if (!getTabTrace()->currentWidget()) return;

	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	if (ui->qFilterLineEdit->text().size() == 0)
		return;

	QString text = ui->qFilterLineEdit->text();
	conn->appendToStringFilters(text, true, true);

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

	conn->recompileStrings();
}

void MainWindow::appendToSearchHistory (QString const & str)
{
	if (str.length() == 0)
		return;
	m_config.m_search_history.insert(str);
	m_config.saveSearchHistory();
	updateSearchHistory();
	ui->qSearchComboBox->setCurrentIndex(ui->qSearchComboBox->findText(str));
}

void MainWindow::updateSearchHistory ()
{
	ui->qSearchComboBox->clear();
	for (size_t i = 0, ie = m_config.m_search_history.size(); i < ie; ++i)
		ui->qSearchComboBox->addItem(m_config.m_search_history[i]);
}

void MainWindow::onEditFindNext ()
{
	if (!getTabTrace()->currentWidget()) return;
	if (Connection * conn = m_server->findCurrentConnection())
		conn->findNext(ui->qSearchComboBox->currentText());
}

void MainWindow::onEditFindPrev ()
{
	if (!getTabTrace()->currentWidget()) return;
	if (Connection * conn = m_server->findCurrentConnection())
		conn->findPrev(ui->qSearchComboBox->currentText());
}




