#include "tablewidget.h"
#include <QScrollBar>
#include "editableheaderview.h"
#include "sparseproxymodel.h"
#include <connection.h>
#include <mainwindow.h>
#include <serialize.h>
#include <utils.h>
#include <utils_qstandarditem.h>
#include <movablelistmodel.h>
#include <delegates.h>
#include <filterproxymodel.h>
#include <find_utils_table.h>
#include "set_with_blocked_signals.h"

namespace table {

	TableWidget::TableWidget (Connection * conn, TableConfig const & cfg, QString const & fname, QStringList const & path)
		: TableView(0), DockedWidgetBase(conn->getMainWindow(), path)
		, m_config(cfg)
		, m_config_ui(m_config, this)
		, m_fname(fname)
		, m_find_widget(0)
		, m_src_model(0)
		, m_proxy_model(0)
		, m_connection(conn)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);

		filterMgr()->m_filter_order.clear();
		filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_String]);
		filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_Ctx]);
		filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_Lvl]);
		filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_Row]);

		m_warnimage = new WarnImage(this);
		m_find_widget = new FindWidget(m_connection->getMainWindow(), this);
		m_find_widget->setActionAbleWidget(this);
		m_find_widget->setParent(this);
		//m_colorize_widget = new ColorizeWidget(m_connection->getMainWindow(), this);
		//m_colorize_widget->setActionAbleWidget(this);
		//m_colorize_widget->setParent(m_tableview);

		MyListModel * model = new MyListModel(this);
		m_config_ui.ui()->columnView->setModel(model);
		/*******************************************************************************************/
		// QT5 deprecated stuff
		/*m_config_ui.ui()->columnView->model()->setSupportedDragActions(Qt::MoveAction);*/
		/*******************************************************************************************/
		m_config_ui.ui()->columnView->setDropIndicatorShown(true);
		m_config_ui.ui()->columnView->setMovement(QListView::Snap);
		m_config_ui.ui()->columnView->setDragDropMode(QAbstractItemView::InternalMove);

		connect(m_config_ui.ui()->columnView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtColumnSetup(QModelIndex)));
		connect(m_config_ui.ui()->autoScrollCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCtxMenuAutoScrollStateChanged(int)));
		connect(m_config_ui.ui()->sparseCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCtxMenuSparseStateChanged(int)));
		connect(m_config_ui.ui()->filteringEnabledCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCtxMenuFilteringStateChanged(int)));
		connect(m_config_ui.ui()->syncGroupSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onCtxMenuSyncGroupChanged(int)));

		//model->addObserver(ui_settings->listViewColumnSizes->model());
		m_config_ui.ui()->columnView->setEditTriggers(QAbstractItemView::NoEditTriggers);

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
		connect(this, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(onTableDoubleClicked(QModelIndex const &)));
		connect(m_config_ui.ui()->sparseCheckBox, SIGNAL(stateChanged(int)), this, SLOT(sparseStateChanged(int)));

		//setHorizontalHeader(new EditableHeaderView(Qt::Horizontal, this));

		m_src_model = new TableModel(this, colorizerMgr(), m_config.m_hhdr, m_config.m_hsize);
		//setModel(m_src_model);
		// TMP!
		//setEditTriggers(QAbstractItemView::NoEditTriggers);
		//setSelectionBehavior(QAbstractItemView::SelectRows);
		//setSelectionMode(QAbstractItemView::SingleSelection);
		//verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
		//horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

		verticalHeader()->setDefaultSectionSize(16);
		horizontalHeader()->setDefaultSectionSize(64);
		horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

		if (!m_proxy_model)
		{
			m_proxy_model = new FilterProxyModel(this, filterMgr(), m_src_model);
			m_proxy_model->setSourceModel(m_src_model);
		}

		verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance

		//setConfigValuesToUI(m_config);
		//onApplyButton();
		applyConfig(m_config);
		setUpdatesEnabled(true);
		horizontalHeader()->setSectionsMovable(true);
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

		setItemDelegate(new SyncedTableItemDelegate(this));

		connect(&getSyncWidgets(), SIGNAL( requestSynchronization(E_SyncMode, int, unsigned long long, void *) ),
							 this, SLOT( performSynchronization(E_SyncMode, int, unsigned long long, void *) ));
		connect(this, SIGNAL( requestSynchronization(E_SyncMode, int, unsigned long long, void *) ),
							 &getSyncWidgets(), SLOT( performSynchronization(E_SyncMode, int, unsigned long long, void *) ));
	}

	TableWidget::~TableWidget ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
	}

	void TableWidget::onShow ()
	{
		show();
	}

	void TableWidget::onHide ()
	{
		hide();
	}

	bool TableWidget::handleAction (Action * a, E_ActionHandleType sync)
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
			case e_Colorize:
			{
				/*if (a->m_args.size() > 0)
				{
					if (a->m_args.at(0).canConvert<ColorizeConfig>())
					{
						ColorizeConfig const cc = a->m_args.at(0).value<ColorizeConfig>();
						handleColorizeAction(cc);
						m_config.m_colorize_config = cc;
						// m_config.save
					}
					return true;
				}*/
			}

			default:
				return false;
		}
		return false;
	}

	void TableWidget::onFilterChanged ()
	{
		onInvalidateFilter();
	}

	void TableWidget::setVisible (bool visible)
	{
		m_dockwidget->setVisible(visible);
		QTableView::setVisible(visible);
	}

	void TableWidget::applyConfig ()
	{
		applyConfig(m_config);
	}

	void TableWidget::applyConfig (TableConfig & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();

		filterMgr()->disconnectFiltersTo(this);
		colorizerMgr()->disconnectFiltersTo(this);


		setModel(m_src_model);
		m_src_model->setProxy(0);

		QStandardItem * name_root = static_cast<QStandardItemModel *>(ui->columnView->model())->invisibleRootItem();
		QStandardItem * size_root = static_cast<QStandardItemModel *>(ui->columnView->model())->invisibleRootItem();
		int const n_cols = name_root->rowCount();
		if (cfg.m_hhdr.size() < n_cols)
			cfg.m_hhdr.resize(n_cols);
		if (cfg.m_hsize.size() < n_cols)
			cfg.m_hsize.resize(n_cols);

		for (int i = 0, ie = name_root->rowCount(); i < ie; ++i)
		{
			QStandardItem * ch = name_root->child(i);
			if (ch && ch->checkState() == Qt::Checked)
			{
				cfg.m_hhdr[i] = ch->text();

				bool const in_pxy = static_cast<SparseProxyModel const *>(m_proxy_model)->colFromSource(i) != -1;
				if (m_proxy_model)
					if (!in_pxy)
					{
						qDebug("col %i not in pxy, adding", i);
						static_cast<SparseProxyModel *>(m_proxy_model)->insertAllowedColumn(i);

					}

				if (cfg.m_hsize[i] == 0)
					cfg.m_hsize[i] = 64;
			}
			else
			{
				cfg.m_hsize[i] = 0;
				if (m_proxy_model)
				{
					static_cast<SparseProxyModel *>(m_proxy_model)->removeAllowedColumn(i);
				}
			}

			//qDebug("apply ui->cfg [%i]: %s sz=%d", i, ch->text(), cfg.m_hsize[i]);
		}

		if (cfg.m_filtering_enabled)
		{
			static_cast<SparseProxyModel *>(m_proxy_model)->force_update();
			m_src_model->setProxy(m_proxy_model);
			setModel(m_proxy_model);
		}
		else
		{
			setModel(m_src_model);
			m_src_model->setProxy(0);
		}

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

		if (colorizerMgr()->getColorizerString())
			colorizerMgr()->getColorizerString()->setSrcModel(m_src_model);
		if (colorizerMgr()->getColorizerRegex())
			colorizerMgr()->getColorizerRegex()->setSrcModel(m_src_model);
		if (colorizerMgr()->getColorizerRow())
			colorizerMgr()->getColorizerRow()->setSrcModel(m_src_model);


		static_cast<SparseProxyModel *>(m_proxy_model)->force_update();

		qDebug("hHeader.size=%i", horizontalHeader()->count());
		for (size_t i = 0, ie = cfg.m_hsize.size(); i < ie; ++i)
		{
			//qDebug("appl table: hdr[%i]=%i", i, cfg.m_hsize.at(i));
			horizontalHeader()->blockSignals(1);
			if (isModelProxy())
			{
				int const src_i = static_cast<SparseProxyModel *>(m_proxy_model)->colToSource(static_cast<int>(i));
				if (src_i != -1)
				{
					horizontalHeader()->resizeSection(static_cast<int>(i), cfg.m_hsize.at(src_i));
					//qDebug("appl   pxy: hdr[%i -> src=%i]=%2i\t\t%s", i, src_i, cfg.m_hsize.at(src_i), cfg.m_hhdr.at(src_i).toStdString().c_str());
				}
			}
			else
			{
				horizontalHeader()->resizeSection(static_cast<int>(i), cfg.m_hsize.at(i));
			}
			horizontalHeader()->blockSignals(0);
		}

		horizontalHeader()->setVisible(true);
		m_src_model->emitLayoutChanged();
	}

	void TableWidget::onHideContextMenu ()
	{
		Ui::SettingsTable * ui = m_config_ui.ui();
		disconnect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		disconnect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		m_config_ui.onHideContextMenu();
	}

	void TableWidget::onShowContextMenu (QPoint const & pos)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		QRect widgetRect = geometry();
		m_config_ui.onShowContextMenu(QCursor::pos());
		Ui::SettingsTable * ui = m_config_ui.ui();

		setConfigValuesToUI(m_config);
		connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		//connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(onResetButton()));
		//connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(onDefaultButton()));
	}

	void TableWidget::setUIValuesToConfig (TableConfig & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();
		m_config.m_sparse_table = ui->sparseCheckBox->checkState() == Qt::Checked;
		m_config.m_filtering_enabled = ui->filteringEnabledCheckBox->checkState() == Qt::Checked;
		m_config.m_auto_scroll = ui->autoScrollCheckBox->checkState() == Qt::Checked;
		m_config.m_sync_group = ui->syncGroupSpinBox->value();
	}

	void TableWidget::onApplyButton ()
	{
		setUIValuesToConfig(m_config);
		applyConfig(m_config);
	}

  void TableWidget::onCtxMenuFilteringStateChanged(int state)
	{
		if (m_config.m_sparse_table ^ state)
		{
			//setUIValuesToConfig(m_config);
			m_config.m_filtering_enabled = state;
			applyConfig(m_config);
			//setConfigValuesToUI(m_config);
		}
	}
  void TableWidget::onCtxMenuAutoScrollStateChanged (int state)
  {
		m_config.m_auto_scroll = state == Qt::Checked;
		// scroll to bottom?
  }
  void TableWidget::onCtxMenuSparseStateChanged(int state)
  {
		if (m_config.m_sparse_table ^ state)
		{
			//setUIValuesToConfig(m_config);
			m_config.m_sparse_table = state;
			applyConfig(m_config);
			//setConfigValuesToUI(m_config);
		}
  }
  void TableWidget::onCtxMenuSyncGroupChanged (int value)
  {
    m_config.m_sync_group = value;
  }

	void TableWidget::setConfigValuesToUI (TableConfig const & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();
		
		setCheckedWithBlockedSignals(ui->tableShowCheckBox, cfg.m_show);
		setCheckedWithBlockedSignals(ui->autoScrollCheckBox, cfg.m_auto_scroll);
		setCheckedWithBlockedSignals(ui->sparseCheckBox, cfg.m_sparse_table);
		setCheckedWithBlockedSignals(ui->filteringEnabledCheckBox, cfg.m_filtering_enabled);
		setValueWithBlockedSignals(ui->syncGroupSpinBox, cfg.m_sync_group);

		clearListView(ui->columnView);
		QStandardItem * name_root = static_cast<QStandardItemModel *>(ui->columnView->model())->invisibleRootItem();
		
		for (int i = 0, ie = m_src_model->columnCount(); i < ie; ++i)
		{
			QString name;
			if (i < cfg.m_hhdr.size() && !cfg.m_hhdr.at(i).isEmpty())
			{
				name = cfg.m_hhdr.at(i);
			}
			else
			{
				name = tr("%0").arg(i);
			}

			Qt::CheckState state = Qt::Checked;
			if (isModelProxy())
			{
				if (static_cast<SparseProxyModel const *>(m_proxy_model)->colFromSource(i) == -1)
				{
					qDebug("cfg->ui pxy col=%i NOT in pxy", i);
					state = Qt::Unchecked;
				}
				else
				{
					if (cfg.m_hsize.size() > 0 && cfg.m_hsize[i] == 0)
					{
						qDebug("cfg->ui pxy col=%i     in pxy, but 0 size", i);
						state = Qt::Unchecked;
					}
					else
						qDebug("cfg->ui pxy col=%i     in pxy", i);
				}
			}
			else
			{
				if (horizontalHeader()->sectionSize(i) == 0)
					state = Qt::Unchecked;
			}

			QList<QStandardItem *> lst = addTriRow(name, state);
			name_root->appendRow(lst);
		}
	}

	void TableWidget::onClickedAtColumnSetup (QModelIndex const idx)
	{
		QStandardItem * const item = static_cast<QStandardItemModel *>(m_config_ui.ui()->columnView->model())->itemFromIndex(idx);
		Qt::CheckState const curr = item->checkState();
		item->setCheckState(curr == Qt::Checked ? Qt::Unchecked : Qt::Checked);
	}

	void TableWidget::onSaveButton ()
	{
		/*m_config.m_hsize.clear();
		m_config.m_hsize.resize(m_src_model->columnCount());
		for (int i = 0, ie = m_src_model->columnCount(); i < ie; ++i)
			m_config.m_hsize[i] = horizontalHeader()->sectionSize(i);*/

		//saveConfigForTable(m_config, m_config.m_tag);

	}
	void TableWidget::onResetButton () { setConfigValuesToUI(m_config); }
	void TableWidget::onDefaultButton ()
	{
		TableConfig defaults;
		//defaults.partialLoadFrom(m_config);
		setConfigValuesToUI(defaults);
	}

	void TableWidget::onSectionResized (int c, int old_size, int new_size)
	{
		int const idx = !isModelProxy() ? c : static_cast<SparseProxyModel *>(m_proxy_model)->colToSource(c);
		//qDebug("table: on rsz hdr[%i -> src=%02i ]  %i->%i\t\t%s", c, idx, old_size, new_size, m_config.m_hhdr.at(idx).toStdString().c_str());
		if (idx < 0) return;
		size_t const curr_sz = m_config.m_hsize.size();
		if (idx < curr_sz)
		{
			//qDebug("%s this=0x%08x hsize[%i]=%i", __FUNCTION__, this, idx, new_size);
		}
		else
		{
			m_config.m_hsize.resize(idx + 1);
			m_config.m_hhdr.resize(idx + 1);
			for (size_t i = curr_sz; i < idx + 1; ++i)
				m_config.m_hhdr[i] = 32;
		}
		m_config.m_hsize[idx] = new_size;
	}

	void TableWidget::handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
	{
		if (mode == e_RecvSync)
			m_src_model->handleCommand(cmd, mode);
		else
			m_queue.push_back(cmd);
	}

	bool TableWidget::appendToColorizers (DecodedCommand const & cmd)
	{
		return colorizerMgr()->action(cmd);
	}

	void TableWidget::commitCommands (E_ReceiveMode mode)
	{
		/*size_t const rows = m_queue.size();
		for (size_t r = 0, re = rows; r < re; ++r)
		{
			DecodedCommand const & cmd = m_queue[r];
			appendToColorizers(cmd);
		}*/

		for (int i = 0, ie = m_queue.size(); i < ie; ++i)
		{
			DecodedCommand & cmd = m_queue[i];
			m_src_model->handleCommand(cmd, mode);
			if (0 == m_src_model->rowCount())
				setCurrentIndex(m_src_model->index(0, 0, QModelIndex()));
			// warning: qmodelindex in source does not exist yet here... 
			appendToFilters(cmd); // this does not bother filters
			//appendToColorizers(cmd); // but colorizer needs qmodelindex
		}
		if (m_queue.size())
		{
			m_src_model->commitCommands(mode);
			if (m_config.m_auto_scroll)
				scrollToBottom();
			m_queue.clear();


			// @FIXME: tmp hack to force resize of columns
			if (isModelProxy())
			{
				horizontalHeader()->blockSignals(1);
				for (int i = 0, ie = model()->columnCount(); i < ie; ++i)
				{
					int const idx = static_cast<SparseProxyModel *>(m_proxy_model)->colToSource(i);
					if (idx > -1 && idx < m_config.m_hsize.size() && m_config.m_hsize[idx] != horizontalHeader()->sectionSize(i))
					{
						horizontalHeader()->resizeSection(i, m_config.m_hsize.at(idx));
						qDebug("table: appnd pxy hdr[%i -> src=%02i ]=%2i\t\t%s", i, idx, m_config.m_hsize.at(idx), m_config.m_hhdr.at(idx).toStdString().c_str());
					}
				}
				horizontalHeader()->blockSignals(0);
			}
			else
			{
				horizontalHeader()->blockSignals(1);
				for (size_t i = 0, ie = m_config.m_hsize.size(); i < ie; ++i)
				{
					if (i < m_src_model->columnCount())
						if (m_config.m_hsize[i] != horizontalHeader()->sectionSize(static_cast<int>(i)))
						{
							horizontalHeader()->resizeSection(static_cast<int>(i), m_config.m_hsize.at(i));
							qDebug("table: rsz hdr[%i]=%i", i, m_config.m_hsize.at(i));
						}
				}
				horizontalHeader()->blockSignals(0);
			}

		}
	}

	void TableWidget::autoScrollOff ()
	{
		m_config.m_auto_scroll = false;
	}

	void TableWidget::autoScrollOn ()
	{
		m_config.m_auto_scroll = true;
	}

	void TableWidget::scrollTo (QModelIndex const & index, ScrollHint hint)
	{
		QTableView::scrollTo(index, hint);
	}

	bool TableWidget::isModelProxy () const
	{
		if (0 == model())
			return false;
		return model() == m_proxy_model;
	}

	void TableWidget::onInvalidateFilter ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		if (isModelProxy())
			static_cast<SparseProxyModel *>(m_proxy_model)->force_update();
		else
		{
			m_src_model->emitLayoutChanged();
		}
	}

  //@TODO: old code, look in LogWidget::onTableDoubleClicked
	void TableWidget::onTableDoubleClicked (QModelIndex const & row_index)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		if (m_config.m_sync_group == 0)
			return;	// do not sync groups with zero

		// @TODO dedup
		QModelIndex const curr_idx = currentIndex();
		QModelIndex mod_idx = currentSourceIndex();

		unsigned long long const t = m_src_model->row_stime(mod_idx.row());

		emit requestSynchronization(e_SyncServerTime, m_config.m_sync_group, t, this);
		scrollTo(curr_idx, QAbstractItemView::PositionAtCenter);
	}

  //@TODO: old code, look in LogWidget::findNearestRow4Time
	void TableWidget::findNearestTimeRow (unsigned long long t)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		bool const is_proxy = isModelProxy();
		int closest_i = 0;
		int closest_dist = 1024 * 1024;
		for (int i = 0; i < m_src_model->rowCount(); ++i)
		{
			int const diff = m_src_model->row_ctime(i) - t; //TODO: fixed ctime!!
			int const d = abs(diff);
			bool const row_exists = is_proxy ? static_cast<SparseProxyModel const *>(m_proxy_model)->rowInProxy(i) : true;
			if (row_exists && d < closest_dist)
			{
				closest_i = i;
				closest_dist = d;
			}
		}

		if (is_proxy)
		{
			//qDebug("table: pxy nearest index= %i/%i", closest_i, m_src_model->rowCount());
			QModelIndex const curr = currentIndex();
			QModelIndex const idx = m_src_model->index(closest_i, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
			//qDebug("table: pxy findNearestTime curr=(%i, %i)  new=(%i, %i)", curr.column(), curr.row(), idx.column(), idx.row());

			QModelIndex const pxy_idx = m_proxy_model->mapFromSource(idx);
			QModelIndex valid_pxy_idx = pxy_idx;
			if (!pxy_idx.isValid())
			{
				valid_pxy_idx = static_cast<SparseProxyModel const *>(m_proxy_model)->mapNearestFromSource(idx);
			}
			//qDebug("table: pxy findNearestTime pxy_new=(%i, %i) valid_pxy_new=(%i, %i)", pxy_idx.column(), pxy_idx.row(), valid_pxy_idx.column(), valid_pxy_idx.row());
			scrollTo(valid_pxy_idx, QAbstractItemView::PositionAtCenter);
			selectionModel()->select(valid_pxy_idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		}
		else
		{
			//qDebug("table: nearest index= %i/%i", closest_i, m_src_model->rowCount());
			QModelIndex const curr = currentIndex();
			QModelIndex const idx = m_src_model->index(closest_i, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
			//qDebug("table: findNearestTime curr=(%i, %i)  new=(%i, %i)", curr.column(), curr.row(), idx.column(), idx.row());

			scrollTo(idx, QAbstractItemView::PositionAtCenter);
			selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		}
	}

	void TableWidget::wheelEvent (QWheelEvent * event)
	{
		bool const mod = event->modifiers() & Qt::CTRL;

		if (mod)
		{
			CursorAction const a = event->delta() < 0 ? MoveDown : MoveUp;
			moveCursor(a, Qt::ControlModifier);
			event->accept();
		}
		else
		{
			QTableView::wheelEvent(event);
		}
	}

	QModelIndex TableWidget::currentSourceIndex () const
	{
		QModelIndex current = currentIndex();
		if (isModelProxy())
		{
			current = m_proxy_model->mapToSource(current);
		}
		return current;
	}

	QModelIndex	TableWidget::moveCursor (CursorAction cursor_action, Qt::KeyboardModifiers modifiers)
	{
		autoScrollOff();
		if (modifiers & Qt::ControlModifier)
		{
			if (cursor_action == MoveHome)
			{
				scrollToTop();
				return QModelIndex(); // @FIXME: should return valid value
			}
			else if (cursor_action == MoveEnd)
			{
				scrollToBottom();
				autoScrollOn();
				return QModelIndex(); // @FIXME too
			}
			else
			{
				QModelIndex const idx = QTableView::moveCursor(cursor_action, modifiers);
				return idx;
			}
		}
		else if (modifiers & Qt::AltModifier)
		{
			QModelIndex const curr_idx = QTableView::moveCursor(cursor_action, modifiers);
			if (curr_idx.isValid())
				setCurrentIndex(curr_idx);
			QModelIndex mod_idx = currentSourceIndex();

			unsigned long long const t = m_src_model->row_stime(mod_idx.row());

			emit requestSynchronization(e_SyncServerTime, m_config.m_sync_group, t, this);
			scrollTo(curr_idx, QAbstractItemView::PositionAtCenter);
			//qDebug("table: pxy findNearestTime curr_idx=(%i, %i)  mod_idx=(%i, %i)", curr_idx.column(), curr_idx.row(), mod_idx.column(), mod_idx.row());
			//unsigned long long const t = m_src_model->row_ctime(mod_idx.row()); //@TODO: fixed ctime
			//m_connection->requestTableActionSync(m_config.m_sync_group, t, cursor_action, modifiers, this);
			//scrollTo(curr_idx, QAbstractItemView::PositionAtCenter);
			return curr_idx;
		}
		else
		{
			//int const value = horizontalScrollBar()->value();
			QModelIndex const idx = QTableView::moveCursor(cursor_action, modifiers);
			scrollTo(idx, QAbstractItemView::PositionAtCenter);
			//horizontalScrollBar()->setValue(value);
			return idx;
		}
	}

	void TableWidget::performSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source)
	{
		qDebug("%s syncgrp=%i time=%i", __FUNCTION__, sync_group, time);

		if (this == source)
			return;

		switch (mode)
		{
			case e_SyncClientTime:
				findNearestRow4Time(true, time);
				break;
			case e_SyncServerTime:
				findNearestRow4Time(false, time);
				break;
			case e_SyncFrame:
			case e_SyncSourceRow:
			case e_SyncProxyRow:
			default:
				break;
		}
	}

	QString TableWidget::getCurrentWidgetPath () const
	{
		QString const appdir = m_connection->getMainWindow()->getAppDir();
		QString const logpath = appdir + "/" + m_connection->getAppName() + "/" + m_connection->getCurrPreset() + "/" + g_TableTag + "/" + m_config.m_tag;
		return logpath;
	}

	void TableWidget::loadAuxConfigs()
	{
		QString const logpath = getCurrentWidgetPath();
		m_config.m_find_config.clear();
		loadConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
		m_config.m_colorize_config.clear();
		loadConfigTemplate(m_config.m_colorize_config, logpath + "/" + g_colorizeTag);
		filterMgr()->loadConfig(logpath);
		colorizerMgr()->loadConfig(logpath);
	}

	void TableWidget::saveAuxConfigs()
	{
		QString const logpath = getCurrentWidgetPath();
		saveConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
		saveConfigTemplate(m_config.m_colorize_config, logpath + "/" + g_colorizeTag);
		filterMgr()->saveConfig(logpath);
		colorizerMgr()->saveConfig(logpath);
	}
	void TableWidget::saveFindConfig()
	{
		QString const logpath = getCurrentWidgetPath();
		saveConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
	}
	void TableWidget::saveColorizeConfig()
	{
		QString const logpath = getCurrentWidgetPath();
		saveConfigTemplate(m_config.m_colorize_config, logpath + "/" + g_colorizeTag);
	}

	void TableWidget::loadConfig (QString const & preset_dir)
	{
		QString const tag_backup = m_config.m_tag;
		QString const path = preset_dir + "/" + g_TableTag + "/" + m_config.m_tag + "/";
		m_config.clear();
		bool const loaded = loadConfigTemplate(m_config, path + g_TableFile);
		if (!loaded)
		{
			TableConfig defaults;
			m_config = defaults;
			//defaultConfigFor(m_config);
			m_config.m_tag = tag_backup; // defaultConfigFor destroys tag
			filterMgr()->m_filter_order.clear();
			filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_String]);
			filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_Ctx]);
			filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_Lvl]);
			filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_Row]);
			//filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_Col]);
		}
		loadAuxConfigs();
	}

	void TableWidget::saveConfig (QString const & path)
	{
		QString const tblpath = getCurrentWidgetPath();
		mkPath(tblpath);

		TableConfig tmp = m_config;
		//normalizeConfig(tmp);
		saveConfigTemplate(tmp, tblpath + "/" + g_TableFile);
		saveAuxConfigs();
	}

	void TableWidget::findNearestRow4Time (bool ctime, unsigned long long t)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		bool const is_proxy = isModelProxy();
		int closest_i = 0;
		long long closest_dist = std::numeric_limits<long long>::max();
		for (int i = 0; i < m_src_model->rowCount(); ++i)
		{
			unsigned long long t0 = ctime ? m_src_model->row_ctime(i) : m_src_model->row_stime(i);
			long long const diff = t0 - t;
			long long const d = abs(diff);
			bool const row_exists = is_proxy ? m_proxy_model->rowInProxy(i) : true;
			if (row_exists && d < closest_dist)
			{
				closest_i = i;
				closest_dist = d;
				//qDebug("table: nearest index= %i/%i %llu", closest_i, m_src_model->rowCount(), d);
			}
		}

		if (is_proxy)
		{
			qDebug("table: pxy nearest index= %i/%i", closest_i, m_src_model->rowCount());
			QModelIndex const curr = currentIndex();
			QModelIndex const idx = m_src_model->index(closest_i, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
			//qDebug("table: pxy findNearestTime curr=(%i, %i)  new=(%i, %i)", curr.column(), curr.row(), idx.column(), idx.row());

			QModelIndex const pxy_idx = m_proxy_model->mapFromSource(idx);
			QModelIndex valid_pxy_idx = pxy_idx;
			if (!pxy_idx.isValid())
			{
				valid_pxy_idx = m_proxy_model->mapNearestFromSource(idx);
			}
			//qDebug("table: pxy findNearestTime pxy_new=(%i, %i) valid_pxy_new=(%i, %i)", pxy_idx.column(), pxy_idx.row(), valid_pxy_idx.column(), valid_pxy_idx.row());
			scrollTo(valid_pxy_idx, QAbstractItemView::PositionAtCenter);
			selectionModel()->select(valid_pxy_idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		}
		else
		{
			qDebug("table: nearest index= %i/%i", closest_i, m_src_model->rowCount());
			QModelIndex const curr = currentIndex();
			QModelIndex const idx = m_src_model->index(closest_i, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
			//qDebug("table: findNearestTime curr=(%i, %i)  new=(%i, %i)", curr.column(), curr.row(), idx.column(), idx.row());

			scrollTo(idx, QAbstractItemView::PositionAtCenter);
			selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		}
	}

	void TableWidget::clearAllData ()
	{
		m_proxy_model->clearModelData();
		m_src_model->clearModelData();
	}

	void TableWidget::onClearAllData ()
	{
		clearAllData();
	}

	void TableWidget::keyPressEvent (QKeyEvent * e)
	{
		if (e->type() == QKeyEvent::KeyPress)
		{
			bool const ctrl_ins = (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier && e->key() == Qt::Key_Insert;
			if (e->matches(QKeySequence::Copy) || ctrl_ins)
			{
				//onCopyToClipboard();
				e->accept();
				return;
			}

			bool const ctrl = (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier;
			bool const shift = (e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier;
			bool const alt = (e->modifiers() & Qt::AltModifier) == Qt::AltModifier;
			bool const x = e->key() == Qt::Key_X;
			bool const h = e->key() == Qt::Key_H;
			if (!ctrl && !shift && !alt && x)
			{
				//onExcludeFileLine();
			}

			/*if (e->key() == Qt::Key_Escape)
			{
				if (m_find_widget && m_find_widget->isVisible())
				{
					m_find_widget->onCancel();
					e->accept();
				}
				if (m_colorize_widget && m_colorize_widget->isVisible())
				{
					m_colorize_widget->onCancel();
					e->accept();
				}
			}*/

			if (!ctrl && shift && !alt && e->key() == Qt::Key_Delete)
			{
				onClearAllData();
				e->accept();
			}

			if (e->matches(QKeySequence::Find))
			{
				onFind();
				e->accept();
			}
			if (!ctrl && !shift && !alt && e->key() == Qt::Key_Slash)
			{
				onFind();
				e->accept();
			}
			if (ctrl && shift && e->key() == Qt::Key_F)
			{
				onFindAllRefs();
				e->accept();
			}

			/*if (ctrl && e->key() == Qt::Key_D)
			{
				onColorize();
				e->accept();
			}*/
			if (!ctrl && !shift && !alt && e->key() == Qt::Key_Slash)
			{
				onFind();
				e->accept();
			}
			/*if (ctrl && shift && e->key() == Qt::Key_F)
			{
				onFindAllRefs();
				e->accept();
			}*/


			if (e->matches(QKeySequence::FindNext))
			{
				onFindNext();
				e->accept();
			}
			if (e->matches(QKeySequence::FindPrevious))
			{
				onFindPrev();
				e->accept();
			}
			// findwidget Tab navigation
			if (e->key() == Qt::Key_Tab && m_find_widget && m_find_widget->isVisible())
			{
				m_find_widget->focusNext();
				e->ignore();
				return;
			}
			if (e->key() == Qt::Key_Backtab && m_find_widget && m_find_widget->isVisible())
			{
				m_find_widget->focusPrev();
				e->ignore();
				return;
			}
			// colorizewidget Tab navigation
			if (e->key() == Qt::Key_Tab && m_colorize_widget && m_colorize_widget->isVisible())
			{
				m_colorize_widget->focusNext();
				e->ignore();
				return;
			}
			if (e->key() == Qt::Key_Backtab && m_colorize_widget && m_colorize_widget->isVisible())
			{
				m_colorize_widget->focusPrev();
				e->ignore();
				return;
			}

		}
		QTableView::keyPressEvent(e);
	}

	void TableWidget::onFind()
	{
		//m_find_widget->onCancel();
		//w->setFocusProxy(m_find_widget);
		//m_find_widget->setFocusProxy(w); // dunno what the proxies are for
		//mk_action configure find widget
		FindConfig & cfg = m_config.m_find_config;
		m_find_widget->applyConfig(cfg);

		m_find_widget->onActivate();

		//setFocusProxy(m_find_widget);
		m_find_widget->setFocusProxy(this); // dunno what the proxies are for
	}

	void TableWidget::onFindNext()
	{
		m_find_widget->onFindNext();
	}

	void TableWidget::onFindPrev()
	{
		m_find_widget->onFindPrev();
	}

	void TableWidget::onFindAllRefs()
	{
		m_find_widget->onFindAllRefs();
	}


	void TableWidget::handleFindAction (FindConfig const & fc)
	{
		bool const select_only = !fc.m_refs && !fc.m_clone;

		if (fc.m_regexp)
		{
			if (fc.m_regexp_val.pattern().isEmpty())
				return;
			if (!fc.m_regexp_val.isValid())
				return;
		}

		saveFindConfig();

		if (select_only)
		{
			if (fc.m_next)
				findAndSelectNext(this, fc);
			else if (fc.m_prev)
				findAndSelectPrev(this, fc);
			else
				findAndSelect(this, fc);
		}
		/*else
		{
			LogWidget * result_widget = 0;
			if (fc.m_refs)
			{
				result_widget = mkFindAllRefsLogWidget(fc);
			}
			else // clone
			{
				result_widget = mkFindAllCloneLogWidget(fc);
			}
		}*/
	}

	void TableWidget::showWarningSign ()
	{
		qDebug("end of search");
		m_connection->getMainWindow()->statusBar()->showMessage(tr("End of document!"));

		// flash icon
		QPoint const global = rect().center();
		QPoint const pos(global.x() - m_warnimage->width() / 2, global.y() - m_warnimage->height() / 2);
			m_warnimage->move(pos);
		m_warnimage->show();
		m_warnimage->activateWindow();
		m_warnimage->raise();
		m_warnimage->warningFindNoMoreMatches();
	}

}

