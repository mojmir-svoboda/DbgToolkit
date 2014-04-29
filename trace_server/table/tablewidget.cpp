#include "tablewidget.h"
#include <QScrollBar>
#include "editableheaderview.h"
#include "sparseproxymodel.h"
#include <connection.h>
#include <utils.h>
#include <utils_qstandarditem.h>
#include <movablelistmodel.h>
#include <delegates.h>

namespace table {

	TableWidget::TableWidget (Connection * conn, QString const & fname, QStringList const & path)
		: QTableView(0), DockedWidgetBase(conn->getMainWindow(), path)
		, m_config()
		, m_config_ui(m_config, this)
		, m_fname(fname)
		, m_modelView(0)
		, m_table_view_proxy(0)
		, m_connection(conn)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);

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

		//model->addObserver(ui_settings->listViewColumnSizes->model());
		m_config_ui.ui()->columnView->setEditTriggers(QAbstractItemView::NoEditTriggers);


		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
		connect(this, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(onTableDoubleClicked(QModelIndex const &)));
		connect(m_config_ui.ui()->filteringCheckBox, SIGNAL(stateChanged(int)), this, SLOT(filteringStateChanged(int)));

		//setHorizontalHeader(new EditableHeaderView(Qt::Horizontal, this));

		m_modelView = new TableModel(this, m_config.m_hhdr, m_config.m_hsize);
		//setModel(m_modelView);
		// TMP!
		//setEditTriggers(QAbstractItemView::NoEditTriggers);
		//setSelectionBehavior(QAbstractItemView::SelectRows);
		//setSelectionMode(QAbstractItemView::SingleSelection);
		//verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
		//horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

		verticalHeader()->setDefaultSectionSize(16);
		horizontalHeader()->setDefaultSectionSize(32);
		horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

		if (!m_table_view_proxy)
		{
			m_table_view_proxy = new SparseProxyModel(this);
			m_table_view_proxy->setSourceModel(m_modelView);
		}

		verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance

		setConfigValuesToUI(m_config);
		onApplyButton();
		setUpdatesEnabled(true);
		horizontalHeader()->setSectionsMovable(true);
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

		setItemDelegate(new SyncedTableItemDelegate(this));
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
		return false;
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

		setModel(m_modelView);
		m_modelView->setProxy(0);

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

				bool const in_pxy = static_cast<SparseProxyModel const *>(m_table_view_proxy)->colFromSource(i) != -1;
				if (m_table_view_proxy)
					if (!in_pxy)
					{
						qDebug("col %i not in pxy, adding", i);
						static_cast<SparseProxyModel *>(m_table_view_proxy)->insertAllowedColumn(i);

					}

				if (cfg.m_hsize[i] == 0)
					cfg.m_hsize[i] = 64;
			}
			else
			{
				cfg.m_hsize[i] = 0;
				if (m_table_view_proxy)
				{
					static_cast<SparseProxyModel *>(m_table_view_proxy)->removeAllowedColumn(i);
				}
			}

			//qDebug("apply ui->cfg [%i]: %s sz=%d", i, ch->text(), cfg.m_hsize[i]);
		}

		if (cfg.m_hide_empty)
		{
			static_cast<SparseProxyModel *>(m_table_view_proxy)->force_update();
			m_modelView->setProxy(m_table_view_proxy);
			setModel(m_table_view_proxy);
		}
		else
		{
			setModel(m_modelView);
			m_modelView->setProxy(0);
		}

		static_cast<SparseProxyModel *>(m_table_view_proxy)->force_update();

		qDebug("hHeader.size=%i", horizontalHeader()->count());
		for (size_t i = 0, ie = cfg.m_hsize.size(); i < ie; ++i)
		{
			//qDebug("appl table: hdr[%i]=%i", i, cfg.m_hsize.at(i));
			horizontalHeader()->blockSignals(1);
			if (isModelProxy())
			{
				int const src_i = static_cast<SparseProxyModel *>(m_table_view_proxy)->colToSource(static_cast<int>(i));
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
		m_modelView->emitLayoutChanged();
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

	void TableWidget::filteringStateChanged (int state)
	{
		if (m_config.m_hide_empty ^ state)
		{
			setUIValuesToConfig(m_config);
			m_config.m_hide_empty = state;
			applyConfig(m_config);
			setConfigValuesToUI(m_config);
		}
	}

	void TableWidget::setConfigValuesToUI (TableConfig const & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();
		
		ui->tableShowCheckBox->blockSignals(true);
		ui->tableShowCheckBox->setCheckState(cfg.m_show ? Qt::Checked : Qt::Unchecked);
		ui->tableShowCheckBox->blockSignals(false);

		ui->filteringCheckBox->blockSignals(true);
		ui->filteringCheckBox->setCheckState(cfg.m_hide_empty ? Qt::Checked : Qt::Unchecked);
		ui->filteringCheckBox->blockSignals(false);

		ui->autoScrollCheckBox->blockSignals(true);
		ui->autoScrollCheckBox->setCheckState(cfg.m_auto_scroll ? Qt::Checked : Qt::Unchecked);
		ui->autoScrollCheckBox->blockSignals(false);

		ui->syncGroupSpinBox->blockSignals(true);
		ui->syncGroupSpinBox->setValue(cfg.m_sync_group);
		ui->syncGroupSpinBox->blockSignals(false);

		clearListView(ui->columnView);
		QStandardItem * name_root = static_cast<QStandardItemModel *>(ui->columnView->model())->invisibleRootItem();
		
		for (int i = 0, ie = m_modelView->columnCount(); i < ie; ++i)
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
				if (static_cast<SparseProxyModel const *>(m_table_view_proxy)->colFromSource(i) == -1)
				{
					qDebug("cfg->ui pxy col=%i NOT in pxy", i);
					state = Qt::Unchecked;
				}
				else
				{
					if (cfg.m_hsize[i] == 0)
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

	void TableWidget::setUIValuesToConfig (TableConfig & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();
		m_config.m_hide_empty = ui->filteringCheckBox->checkState() == Qt::Checked;
		m_config.m_auto_scroll = ui->autoScrollCheckBox->checkState() == Qt::Checked;
		m_config.m_sync_group = ui->syncGroupSpinBox->value();
	}

	void TableWidget::onApplyButton ()
	{
		setUIValuesToConfig(m_config);
		applyConfig(m_config);
	}

	void TableWidget::onSaveButton ()
	{
		/*m_config.m_hsize.clear();
		m_config.m_hsize.resize(m_modelView->columnCount());
		for (int i = 0, ie = m_modelView->columnCount(); i < ie; ++i)
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
		int const idx = !isModelProxy() ? c : static_cast<SparseProxyModel *>(m_table_view_proxy)->colToSource(c);
		qDebug("table: on rsz hdr[%i -> src=%02i ]  %i->%i\t\t%s", c, idx, old_size, new_size, m_config.m_hhdr.at(idx).toStdString().c_str());
		if (idx < 0) return;
		size_t const curr_sz = m_config.m_hsize.size();
		if (idx < curr_sz)
		{
			//qDebug("%s this=0x%08x hsize[%i]=%i", __FUNCTION__, this, idx, new_size);
		}
		else
		{
			m_config.m_hsize.resize(idx + 1);
			for (size_t i = curr_sz; i < idx + 1; ++i)
				m_config.m_hsize[i] = 32;
		}
		m_config.m_hsize[idx] = new_size;
	}

	void TableWidget::appendTableXY (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & msg)
	{
		m_modelView->appendTableXY(x, y, time, fgc, bgc, msg);

		if (m_config.m_auto_scroll)
			scrollToBottom();

		// @FIXME: tmp hack to force resize of columns
		if (isModelProxy())
		{
			horizontalHeader()->blockSignals(1);
			for (int i = 0, ie = model()->columnCount(); i < ie; ++i)
			{
				int const idx = static_cast<SparseProxyModel *>(m_table_view_proxy)->colToSource(i);
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
				if (i < m_modelView->columnCount())
					if (m_config.m_hsize[i] != horizontalHeader()->sectionSize(static_cast<int>(i)))
					{
						horizontalHeader()->resizeSection(static_cast<int>(i), m_config.m_hsize.at(i));
						qDebug("table: rsz hdr[%i]=%i", i, m_config.m_hsize.at(i));
					}
			}
			horizontalHeader()->blockSignals(0);
		}
	}

	void TableWidget::appendTableSetup (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & hhdr, QString const & tag)
	{
		unsigned long long t = time.toULongLong();
		if (!hhdr.isEmpty() && x > -1)
		{
			m_modelView->setHeaderData(x, Qt::Horizontal, hhdr, Qt::EditRole);
			qDebug("header h[x=%2i, y=%2i] = %s", x, y, hhdr.toStdString().c_str());
		}

		if (!fgc.isEmpty() && x >= 0 && y >= 0)
		{
			m_modelView->createRows(t, y, y, QModelIndex());
			m_modelView->createColumns(t, x, x, QModelIndex());
			QModelIndex const & idx = m_modelView->index(y, x, QModelIndex());
			m_modelView->setData(idx, QColor(fgc), Qt::ForegroundRole);
		}

		if (!bgc.isEmpty() && x >= 0 && y >= 0)
		{
			m_modelView->createRows(t, y, y, QModelIndex());
			m_modelView->createColumns(t, x, x, QModelIndex());
			QModelIndex const & idx = m_modelView->index(y, x, QModelIndex());
			m_modelView->setData(idx, QColor(bgc), Qt::BackgroundRole);
		}
	}

	void TableWidget::scrollTo (QModelIndex const & index, ScrollHint hint)
	{
		QTableView::scrollTo(index, hint);
	}

	bool TableWidget::isModelProxy () const
	{
		if (0 == model())
			return false;
		return model() == m_table_view_proxy;
	}

	void TableWidget::onInvalidateFilter ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		if (isModelProxy())
			static_cast<SparseProxyModel *>(m_table_view_proxy)->force_update();
		else
		{
			m_modelView->emitLayoutChanged();
		}
	}

  //@TODO: old code, look in LogWidget::onTableDoubleClicked
	void TableWidget::onTableDoubleClicked (QModelIndex const & row_index)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		if (m_config.m_sync_group == 0)
			return;	// do not sync groups with zero

		if (isModelProxy())
		{
			QModelIndex const curr = m_table_view_proxy->mapToSource(row_index);
			unsigned long long const time = m_modelView->row_ctime(curr.row());

			qDebug("table: dblClick (pxy) curr=(%i, %i)  time=%llu", curr.row(), curr.column(), time);
      //@TODO: old call!!
			m_connection->requestTableSynchronization(m_config.m_sync_group, time);
		}
		else
		{
			QModelIndex const curr = row_index;
			unsigned long long const time = m_modelView->row_ctime(curr.row());
			qDebug("table: dblClick curr=(%i, %i)  time=%llu", curr.row(), curr.column(), time);
			m_connection->requestTableSynchronization(m_config.m_sync_group, time);
		}
	}

  //@TODO: old code, look in LogWidget::findNearestRow4Time
	void TableWidget::findNearestTimeRow (unsigned long long t)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		bool const is_proxy = isModelProxy();
		int closest_i = 0;
		int closest_dist = 1024 * 1024;
		for (int i = 0; i < m_modelView->rowCount(); ++i)
		{
			int const diff = m_modelView->row_ctime(i) - t; //TODO: fixed ctime!!
			int const d = abs(diff);
			bool const row_exists = is_proxy ? static_cast<SparseProxyModel const *>(m_table_view_proxy)->rowInProxy(i) : true;
			if (row_exists && d < closest_dist)
			{
				closest_i = i;
				closest_dist = d;
			}
		}

		if (is_proxy)
		{
			//qDebug("table: pxy nearest index= %i/%i", closest_i, m_modelView->rowCount());
			QModelIndex const curr = currentIndex();
			QModelIndex const idx = m_modelView->index(closest_i, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
			//qDebug("table: pxy findNearestTime curr=(%i, %i)  new=(%i, %i)", curr.column(), curr.row(), idx.column(), idx.row());

			QModelIndex const pxy_idx = m_table_view_proxy->mapFromSource(idx);
			QModelIndex valid_pxy_idx = pxy_idx;
			if (!pxy_idx.isValid())
			{
				valid_pxy_idx = static_cast<SparseProxyModel const *>(m_table_view_proxy)->mapNearestFromSource(idx);
			}
			//qDebug("table: pxy findNearestTime pxy_new=(%i, %i) valid_pxy_new=(%i, %i)", pxy_idx.column(), pxy_idx.row(), valid_pxy_idx.column(), valid_pxy_idx.row());
			scrollTo(valid_pxy_idx, QAbstractItemView::PositionAtCenter);
			selectionModel()->select(valid_pxy_idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		}
		else
		{
			//qDebug("table: nearest index= %i/%i", closest_i, m_modelView->rowCount());
			QModelIndex const curr = currentIndex();
			QModelIndex const idx = m_modelView->index(closest_i, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
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

	void TableWidget::requestTableWheelEventSync (QWheelEvent * ev, QTableView const * source)
	{
		// obsolette
	}

	QModelIndex	TableWidget::moveCursor (CursorAction cursor_action, Qt::KeyboardModifiers modifiers)
	{
		bool const mod = modifiers & Qt::CTRL;
		if (!mod)
			return QTableView::moveCursor(cursor_action, modifiers);
		else
		{
			QModelIndex const curr_idx = QTableView::moveCursor(cursor_action, modifiers);
			if (curr_idx.isValid())
				setCurrentIndex(curr_idx);
			QModelIndex mod_idx = curr_idx;
			if (isModelProxy())
				mod_idx = m_table_view_proxy->mapToSource(curr_idx);

			unsigned long long const t = m_modelView->row_ctime(mod_idx.row()); //@TODO: fixed ctime
			m_connection->requestTableActionSync(m_config.m_sync_group, t, cursor_action, modifiers, this);
			scrollTo(curr_idx, QAbstractItemView::PositionAtCenter);
			//qDebug("table: pxy findNearestTime curr_idx=(%i, %i)  mod_idx=(%i, %i)", curr_idx.column(), curr_idx.row(), mod_idx.column(), mod_idx.row());
			return curr_idx;
		}
	}

	void TableWidget::requestTableActionSync (unsigned long long t, int cursor_action, Qt::KeyboardModifiers modifiers, QTableView const * source)
	{
		if (this != source)
			findNearestTimeRow(t);
	}

	void TableWidget::loadConfig (QString const & path)
	{
	}

	void TableWidget::saveConfig (QString const & path)
	{
	}

	void TableWidget::commitCommands (E_ReceiveMode mode)
	{
	}
}


