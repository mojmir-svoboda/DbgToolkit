#include "logwidget.h"
//#include <QScrollBar>
#include <QClipboard>
#include <connection.h>
#include <utils.h>
#include <utils_qstandarditem.h>
#include "logdelegate.h"
#include <syncwidgets.h>
#include "logtablemodel.h"
#include "filterproxymodel.h"
#include "findproxymodel.h"
#include "warnimage.h"
#include <QInputDialog>
#include <QFontDialog>


namespace logs {

	LogWidget::LogWidget (Connection * connection, QWidget * wparent, LogConfig & cfg, QString const & fname, QStringList const & path)
		: TableView(wparent), ActionAble(path)
		, m_connection(connection)
		, m_config(cfg)
		, m_config2(cfg)
		, m_config_ui(*this, this)
		, m_fname(fname)
		, m_tab(0)
		, m_linked_parent(0)
		, m_dwb(0)
		, m_warnimage(0)
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
		, m_proxy_model(0)
		, m_find_proxy_model(0)
		, m_src_model(0)
		, m_selection(0)
		, m_ksrc_selection(0)
		, m_kproxy_selection(0)
		, m_src_selection(0)
		, m_proxy_selection(0)
		, m_find_proxy_selection(0)
		, m_kfind_proxy_selection(0)
		, m_color_regex_model(0)
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

		setStyleSheet("QTableView::item{ selection-background-color:	#F5DEB3  } QTableView::item{ selection-color: #000000 }");
		
		// to ignore 'resizeColumnToContents' when accidentaly double-clicked on header handle
		QObject::disconnect(horizontalHeader(), SIGNAL(sectionHandleDoubleClicked(int)), this, SLOT(resizeColumnToContents(int)));

		setObjectName(QString::fromUtf8("tableView"));
		//setupThreadColors(connection->getMainWindow()->getThreadColors());


		QStyle const * const style = QApplication::style();
		m_config_ui.ui()->findWidget->setMainWindow(m_connection->getMainWindow());
		connect(m_config_ui.ui()->gotoNextButton, SIGNAL(clicked()), this, SLOT(onNextToView()));
		m_config_ui.ui()->gotoNextButton->setIcon(style->standardIcon(QStyle::SP_ArrowDown));
		connect(m_config_ui.ui()->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		//connect(ui->autoScrollCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onAutoScrollStateChanged(int)));
		//connect(ui->inViewCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onInViewStateChanged(int)));
		//connect(m_config_ui.ui()->filterFileCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onFilterFile(int)));
		connect(this, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onTableClicked(QModelIndex const &)));
		//connect(this, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(onTableDoubleClicked(QModelIndex const &)));

		//setupColorRegex();

		QObject::connect(horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));
		QObject::connect(horizontalHeader(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(onSectionMoved(int, int, int)));
		verticalHeader()->setFont(cfg.m_font);
		verticalHeader()->setDefaultSectionSize(cfg.m_row_width);
		verticalHeader()->hide(); // @NOTE: users want that //@NOTE2: they can't have it because of performance
		horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		setItemDelegate(new LogDelegate(*this, m_connection->appData(), this));
		
		m_warnimage = new WarnImage(this);
	}

	void LogWidget::setupNewLogModel ()
	{
		setupLogModel(0);
	}

	void LogWidget::setupLogModel (LogTableModel * linked_model)
	{
		if (linked_model)
			m_src_model = linked_model;
		else
		{
			m_src_model = new LogTableModel(this, *this);
			QObject::disconnect(m_src_model, SIGNAL(rowsInserted(QModelIndex,int,int)), verticalHeader(), SLOT(sectionsInserted(QModelIndex,int,int)));
		}

		m_proxy_model = new FilterProxyModel(this, *this);
		m_proxy_model->setSourceModel(m_src_model);

		m_find_proxy_model = new FindProxyModel(this, *this);
		m_find_proxy_model->setSourceModel(m_src_model);

		setupLogSelectionProxy();
	}

	void LogWidget::setupLogSelectionProxy ()
	{
		m_src_selection = new QItemSelectionModel(m_src_model);
		m_proxy_selection = new QItemSelectionModel(m_proxy_model);
		m_ksrc_selection = new KLinkItemSelectionModel(m_src_model, m_src_selection);
		m_kproxy_selection = new KLinkItemSelectionModel(m_proxy_model, m_proxy_selection);
		//setSelectionModel(m_src_selection);
		//m_selection = new LogSelectionProxyModel(m_src_model, m_src_selection);
	}

	LogWidget::~LogWidget ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		for (linked_widgets_t::iterator it = m_linked_widgets.begin(), ite = m_linked_widgets.end(); it != ite; ++it)
		{
			DockedWidgetBase * child = *it;
			m_connection->destroyDockedWidget(child);
		}
		m_linked_widgets.clear();

	/*	if (m_file_csv_stream)
		{
			QIODevice * const f = m_file_csv_stream->device();
			f->close();
			delete m_file_csv_stream;
			m_file_csv_stream = 0;
			delete f;
		}*/
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
		disconnect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		m_config_ui.onHideContextMenu();
	}

	void LogWidget::onShowContextMenu (QPoint const & pos)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		m_config_ui.onShowContextMenu(QCursor::pos());
		Ui::SettingsLog * ui = m_config_ui.ui();

		setConfigValuesToUI(m_config);
		//connect(ui->logViewComboBox, SIGNAL(activated(int)), this, SLOT(onLogViewActivate(int)));
	}

	void LogWidget::swapSectionsAccordingTo (logs::LogConfig const & cfg)
	{
		QStringList src;
		int const hn = horizontalHeader()->count();
		for (int hi = 0; hi < hn; ++hi)
		{
			int const li = horizontalHeader()->logicalIndex(hi);
			QString const li_str = m_config.m_columns_setup[li]; // @note: from m_config
			src << li_str;
		}
	
		QStringList tgt;
		int const nn = cfg.m_columns_setup.size();
		for (int nj = 0; nj < nn; ++nj)
		{
			tgt << cfg.m_columns_setup[nj];
		}

		for (int nj = 0; nj < nn; ++nj)
			for (int hi = 0; hi < hn; ++hi)
				if (src[hi] == tgt[nj])
				{
					if (hi != nj)
					{
						std::swap(src[hi], src[nj]);
						horizontalHeader()->swapSections(hi, nj);
						break;
					}
				}

		//void LogWidget::resizeSections ()
		{
			bool const old = blockSignals(true);
			for (int c = 0, ce = cfg.m_columns_sizes.size(); c < ce; ++c)
			{
				int const li = horizontalHeader()->logicalIndex(c);
				horizontalHeader()->resizeSection(li, cfg.m_columns_sizes.at(c));
			}
			blockSignals(old);
		}
	}

	void LogWidget::applyConfig ()
	{
		filterMgr()->disconnectFiltersTo(this);
		colorizerMgr()->disconnectFiltersTo(this);

		swapSectionsAccordingTo(m_config);
		applyConfig(m_config);
		filterMgr()->applyConfig();
		colorizerMgr()->applyConfig();

		filterMgr()->connectFiltersTo(this);
		colorizerMgr()->connectFiltersTo(this);

		connect(filterMgr(), SIGNAL(filterEnabledChanged()), this, SLOT(onFilterEnabledChanged()));
		connect(filterMgr(), SIGNAL(filterChangedSignal()), this, SLOT(onInvalidateFilter()));
		// @TODO: nesel by mensi brutus nez je invalidate filter?
		connect(colorizerMgr(), SIGNAL(filterEnabledChanged()), this, SLOT(onFilterEnabledChanged()));
		connect(colorizerMgr(), SIGNAL(filterChangedSignal()), this, SLOT(onInvalidateFilter()));

		if (filterMgr()->getFilterCtx())
			filterMgr()->getFilterCtx()->setAppData(&m_connection->appData());

		//if (colorizerMgr()->getFilterCtx())
		//	colorizerMgr()->getFilterCtx()->setAppData(&m_connection->appData());
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
		int const idx = !isModelProxy() ? column : m_proxy_model->colToSource(column);
		//qDebug("table: on rsz hdr[%i -> src=%02i ]	%i->%i\t\t%s", c, idx, old_size, new_size, m_config.m_hhdr.at(idx).toStdSt
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

	QString LogWidget::getCurrentWidgetPath () const
	{
		QString const appdir = m_connection->getMainWindow()->getAppDir();
		QString const logpath = appdir + "/" + m_connection->getCurrPreset() + "/" + g_LogTag + "/" + m_config.m_tag;
		return logpath;
	}

	void LogWidget::loadConfig (QString const & preset_dir)
	{
		QString const logpath = preset_dir + "/" + g_LogTag + "/" + m_config.m_tag + "/";
		m_config.clear();
		bool const loaded = logs::loadConfig(m_config, logpath + g_LogFile);
		if (!loaded)
			defaultConfigFor(m_config);
		
		loadAuxConfigs();
	}
	void LogWidget::loadAuxConfigs ()
	{
		QString const logpath = getCurrentWidgetPath();
		m_config.m_find_config.clear();
		loadConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
		filterMgr()->loadConfig(logpath);
		colorizerMgr()->loadConfig(logpath);
	}
	void LogWidget::saveAuxConfigs ()
	{
		QString const logpath = getCurrentWidgetPath();
		saveConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
		filterMgr()->saveConfig(logpath);
		colorizerMgr()->saveConfig(logpath);
	}
	void LogWidget::saveFindConfig ()
	{
		QString const logpath = getCurrentWidgetPath();
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
		QString const logpath = getCurrentWidgetPath();
		mkDir(logpath);

		logs::LogConfig tmp = m_config;
		normalizeConfig(tmp);
		logs::saveConfig(tmp, logpath + "/" + g_LogFile);
		saveAuxConfigs();
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

	void LogWidget::onFilterEnabledChanged ()
	{
		qDebug("%s", __FUNCTION__);
		applyConfig(m_config);
		//setupUi
		//applyConfig();
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
		appendToColorizers(cmd);
	}
	if (m_queue.size())
	{
		m_src_model->commitCommands(mode);
		if (m_config.m_auto_scroll)
			scrollToBottom();
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

QString LogWidget::exportSelection ()
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
		QString const simplified = m->data(current).toString().simplified();
		selected_text.append(simplified);
		
		if (i + 1 < indexes.size() && current.row() != indexes.at(i + 1).row())
			selected_text.append('\n'); // switching rows
		else
			selected_text.append('\t');
	}
	return selected_text;
}

void LogWidget::onCopyToClipboard ()
{
	QString const text = exportSelection();
	QClipboard * clipboard = QApplication::clipboard();
	clipboard->setText(text);
}

void LogWidget::keyPressEvent (QKeyEvent * e)
{
	if (e->type() == QKeyEvent::KeyPress)
	{
		bool const ctrl_ins = (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier && e->key() == Qt::Key_Insert;
		if (e->matches(QKeySequence::Copy) || ctrl_ins)
		{
			onCopyToClipboard();
			e->accept();
			return;
		}

		bool const ctrl = (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier;
		bool const shift = (e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier;
		bool const alt = (e->modifiers() & Qt::AltModifier) == Qt::AltModifier;
		bool const x = e->key() == Qt::Key_X;
		if (!ctrl && !shift && !alt && x)
		{
			onExcludeFileLine();
		}

	}
	QTableView::keyPressEvent(e);
}

void LogWidget::scrollTo (QModelIndex const & index, ScrollHint hint)
{
	QTableView::scrollTo(index, hint);
}

void LogWidget::autoScrollOff ()
{
	m_config.m_auto_scroll = false;
}

void LogWidget::autoScrollOn ()
{
	m_config.m_auto_scroll = true;
}

QModelIndex LogWidget::moveCursor (CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
	autoScrollOff();
	if (modifiers & Qt::ControlModifier)
	{
		if (cursorAction == MoveHome)
		{
			scrollToTop();
			return QModelIndex(); // @FIXME: should return valid value
		}
		else if (cursorAction == MoveEnd)
		{
			scrollToBottom();
			autoScrollOn();
			return QModelIndex(); // @FIXME too
		}
		else
			return QTableView::moveCursor(cursorAction, modifiers);
	}
	else
		return QTableView::moveCursor(cursorAction, modifiers);

	/*int const value = horizontalScrollBar()->value();
	QModelIndex const ret = QTableView::moveCursor(cursorAction, modifiers);
	horizontalScrollBar()->setValue(value);
	return ret;*/
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

void LogWidget::onSectionMoved (int logical, int old_visual, int new_visual)
{
	qDebug("log: section moved logical=%i old_visual=%i new_visual=%i", logical, old_visual, new_visual);
}

void LogWidget::onSectionResized (int logical, int old_size, int new_size)
{
	qDebug("log: section resized logical=%i old_sz=%i new_sz=%i", logical, old_size, new_size);
	int const idx = !isModelProxy() ? logical : m_proxy_model->colToSource(logical);
	//qDebug("table: on rsz hdr[%i -> src=%02i ]	%i->%i\t\t%s", c, idx, old_size, new_size, m_config.m_hhdr.at(idx).toStdString().c_str());
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
			QString simplified = txt.simplified();
			QString const quoted_txt = simplified.replace(regex, to_string);
			str << "\"" << quoted_txt << "\"";
			if (c < ce - 1)
				str << ",\t";
		}
		str << "\n";
	}
	csv.close();
}


}


