#include "basetable.h"
#include <QTimer>
#include <QScrollBar>
#include "editableheaderview.h"
#include "sparseproxy.h"
#include "connection.h"
#include "utils.h"
#include "utils_qstandarditem.h"
#include "movablelistmodel.h"

namespace table {

	BaseTable::BaseTable (Connection * oparent, QWidget * wparent, TableConfig & cfg, QString const & fname)
		: QTableView(wparent)
		, m_timer(-1)
		, m_config(cfg)
		, m_config_ui(cfg, this)
		, m_fname(fname)
		, m_modelView(0)
		, m_table_view_proxy(0)
		, m_connection(oparent)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);

		MyListModel * model = new MyListModel(this);
		m_config_ui.ui()->columnView->setModel(model);
		m_config_ui.ui()->columnView->model()->setSupportedDragActions(Qt::MoveAction);
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

		m_modelView = new TableModelView(this, m_config.m_hhdr, m_config.m_hsize);
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
		QTimer::singleShot(0, this, SLOT(onApplyButton()));
		setUpdatesEnabled(true);
		horizontalHeader()->setSectionsMovable(true);
	}

	BaseTable::~BaseTable ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		stopUpdate();
		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
	}

	void BaseTable::onShow ()
	{
		show();
	}

	void BaseTable::onHide ()
	{
		hide();
	}

	void BaseTable::applyConfig (TableConfig & cfg)
	{
		// FIXME: for unknown reasons this fn is called multiple times (from onApplyButton)
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();

		setModel(m_modelView);
		m_modelView->setProxy(0);

		QStandardItem * name_root = static_cast<QStandardItemModel *>(ui->columnView->model())->invisibleRootItem();
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
				if (cfg.m_hsize[i] == 0)
					cfg.m_hsize[i] = 32;
			}
			else
			{
				cfg.m_hsize[i] = 0;
			}
			if (m_table_view_proxy)
				static_cast<SparseProxyModel *>(m_table_view_proxy)->insertAllowedColumn(i);
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

		for (int i = 0, ie = cfg.m_hsize.size(); i < ie; ++i)
		{
			horizontalHeader()->blockSignals(1);
			int const src_i = !isModelProxy() ? i : static_cast<SparseProxyModel *>(m_table_view_proxy)->colFromSource(i);
			horizontalHeader()->resizeSection(i, cfg.m_hsize.at(src_i));
			//qDebug("table: hdr[%i]=%i", i, cfg.m_hsize.at(i));
			horizontalHeader()->blockSignals(0);
		}

		horizontalHeader()->setVisible(true);

		m_modelView->emitLayoutChanged();
	}

	void BaseTable::stopUpdate ()
	{
		if (m_timer != -1)
			killTimer(m_timer);
	}

	void BaseTable::timerEvent (QTimerEvent * e)
	{
		update();
	}

	void BaseTable::onHideContextMenu ()
	{
		Ui::SettingsTable * ui = m_config_ui.ui();
		disconnect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		disconnect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		m_config_ui.onHideContextMenu();
	}

	void BaseTable::onShowContextMenu (QPoint const & pos)
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

	void BaseTable::filteringStateChanged (int state)
	{
		if (m_config.m_hide_empty ^ state)
		{
			setUIValuesToConfig(m_config);
			m_config.m_hide_empty = state;
			applyConfig(m_config);
			setConfigValuesToUI(m_config);
		}
	}

	void BaseTable::setConfigValuesToUI (TableConfig const & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();
		
		ui->tableShowCheckBox->setCheckState(cfg.m_show ? Qt::Checked : Qt::Unchecked);
		ui->filteringCheckBox->setCheckState(cfg.m_hide_empty ? Qt::Checked : Qt::Unchecked);
		ui->autoScrollCheckBox->setCheckState(cfg.m_auto_scroll ? Qt::Checked : Qt::Unchecked);
		ui->syncGroupSpinBox->setValue(cfg.m_sync_group);

		clearListView(ui->columnView);
		QStandardItem * name_root = static_cast<QStandardItemModel *>(ui->columnView->model())->invisibleRootItem();
		
		for (int i = 0, ie = m_modelView->columnCount(); i < ie; ++i)
		{
			Qt::CheckState state = Qt::Checked;
			QString name;
			if (i < cfg.m_hhdr.size() && !cfg.m_hhdr.at(i).isEmpty())
			{
				name = cfg.m_hhdr.at(i);
			}
			else
			{
				name = tr("%0").arg(i);
			}

			if (isModelProxy())
			{
				if (!static_cast<SparseProxyModel const *>(m_table_view_proxy)->colInProxy(i))
				{
					state = Qt::Unchecked;
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

	void BaseTable::onClickedAtColumnSetup (QModelIndex const idx)
	{
		QStandardItem * const item = static_cast<QStandardItemModel *>(m_config_ui.ui()->columnView->model())->itemFromIndex(idx);
		Qt::CheckState const curr = item->checkState();
		item->setCheckState(curr == Qt::Checked ? Qt::Unchecked : Qt::Checked);
	}

	void BaseTable::setUIValuesToConfig (TableConfig & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();
		m_config.m_hide_empty = ui->filteringCheckBox->checkState() == Qt::Checked;
		m_config.m_auto_scroll = ui->autoScrollCheckBox->checkState() == Qt::Checked;
		m_config.m_sync_group = ui->syncGroupSpinBox->value();
	}

	void BaseTable::onApplyButton ()
	{
		setUIValuesToConfig(m_config);
		applyConfig(m_config);
	}

	void BaseTable::onSaveButton ()
	{
		/*m_config.m_hsize.clear();
		m_config.m_hsize.resize(m_modelView->columnCount());
		for (int i = 0, ie = m_modelView->columnCount(); i < ie; ++i)
			m_config.m_hsize[i] = horizontalHeader()->sectionSize(i);*/
		m_connection->saveConfigForTable(m_config, m_config.m_tag);

	}
	void BaseTable::onResetButton () { setConfigValuesToUI(m_config); }
	void BaseTable::onDefaultButton ()
	{
		TableConfig defaults;
		//defaults.partialLoadFrom(m_config);
		setConfigValuesToUI(defaults);
	}

	void BaseTable::onSectionResized (int c, int old_size, int new_size)
	{
		int const idx = !isModelProxy() ? c : static_cast<SparseProxyModel *>(m_table_view_proxy)->colFromSource(c);
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
			//m_modelView->setHeaderData(c, Qt::Horizontal, m_config.m_hsize[i], 
		}
		m_config.m_hsize[idx] = new_size;
	}

	void BaseTable::appendTableXY (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & msg)
	{
		m_modelView->appendTableXY(x, y, time, fgc, bgc, msg);

		if (m_config.m_auto_scroll)
			scrollToBottom();

		// @FIXME: tmp hack to force resize of columns
		if (isModelProxy())
		{
			for (int i = 0, ie = model()->columnCount(); i < ie; ++i)
			{
				int const idx = static_cast<SparseProxyModel *>(m_table_view_proxy)->colFromSource(i);
				if (idx > -1 && m_config.m_hsize[idx] != horizontalHeader()->sectionSize(i))
				{
					horizontalHeader()->blockSignals(1);
					horizontalHeader()->resizeSection(i, m_config.m_hsize.at(idx));
					//qDebug("table: rsz pxy col=%i hdr[%i]=%i", i, idx, m_config.m_hsize.at(idx));
					horizontalHeader()->blockSignals(0);
				}
			}
		}
		else
		{
			if (m_modelView->columnCount() < m_config.m_hsize.size())
				for (int i = 0, ie = m_config.m_hsize.size(); i < ie; ++i)
				{
					if (m_config.m_hsize[i] != horizontalHeader()->sectionSize(i))
					{
						horizontalHeader()->blockSignals(1);
						horizontalHeader()->resizeSection(i, m_config.m_hsize.at(i));
						//qDebug("table: rsz hdr[%i]=%i", i, m_config.m_hsize.at(i));
						horizontalHeader()->blockSignals(0);
					}
				}
		}
	}

	void BaseTable::appendTableSetup (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & hhdr, QString const & tag)
	{
		unsigned long long t = time.toULongLong();
		if (!hhdr.isEmpty() && x > -1)
		{
			m_modelView->setHeaderData(x, Qt::Horizontal, hhdr, Qt::EditRole);
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

	void BaseTable::scrollTo (QModelIndex const & index, ScrollHint hint)
	{
		QTableView::scrollTo(index, hint);
	}

	bool BaseTable::isModelProxy () const
	{
		if (0 == model())
			return false;
		return model() == m_table_view_proxy;
	}

	void BaseTable::onInvalidateFilter ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		if (isModelProxy())
			static_cast<SparseProxyModel *>(m_table_view_proxy)->force_update();
		else
		{
			m_modelView->emitLayoutChanged();
		}
	}

	void BaseTable::onTableDoubleClicked (QModelIndex const & row_index)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		if (m_config.m_sync_group == 0)
			return;	// do not sync groups with zero

		if (isModelProxy())
		{
			QModelIndex const curr = m_table_view_proxy->mapToSource(row_index);
			unsigned long long const time = m_modelView->row_time(curr.row());

			qDebug("table: dblClick (pxy) curr=(%i, %i)  time=%llu", curr.row(), curr.column(), time);
			m_connection->requestTableSynchronization(m_config.m_sync_group, time);
		}
		else
		{
			QModelIndex const curr = row_index;
			unsigned long long const time = m_modelView->row_time(curr.row());
			qDebug("table: dblClick curr=(%i, %i)  time=%llu", curr.row(), curr.column(), time);
			m_connection->requestTableSynchronization(m_config.m_sync_group, time);
		}
	}

	void BaseTable::findNearestTimeRow (unsigned long long t)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		bool const is_proxy = isModelProxy();
		int closest_i = 0;
		int closest_dist = 1024 * 1024;
		for (int i = 0; i < m_modelView->rowCount(); ++i)
		{
			int const diff = m_modelView->row_time(i) - t;
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
			qDebug("table: pxy nearest index= %i/%i", closest_i, m_modelView->rowCount());
			QModelIndex const curr = currentIndex();
			QModelIndex const idx = m_modelView->index(closest_i, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
			qDebug("table: pxy findNearestTime curr=(%i, %i)  new=(%i, %i)", curr.column(), curr.row(), idx.column(), idx.row());

			scrollTo(m_table_view_proxy->mapFromSource(idx), QAbstractItemView::PositionAtCenter);
			selectionModel()->select(m_table_view_proxy->mapFromSource(idx), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		}
		else
		{
			qDebug("table: nearest index= %i/%i", closest_i, m_modelView->rowCount());
			QModelIndex const curr = currentIndex();
			QModelIndex const idx = m_modelView->index(closest_i, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
			qDebug("table: findNearestTime curr=(%i, %i)  new=(%i, %i)", curr.column(), curr.row(), idx.column(), idx.row());

			scrollTo(idx, QAbstractItemView::PositionAtCenter);
			selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		}
	}

	void BaseTable::wheelEvent (QWheelEvent * event)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		bool const shift = event->modifiers() & Qt::ALT;

		QTableView::wheelEvent(event);
		m_connection->requestTableWheelEventSync(m_config.m_sync_group, event, this);
	}

	void BaseTable::requestTableWheelEventSync (QWheelEvent * ev, QTableView const * source)
	{
		//int const hmin = tbl->widget().horizontalScrollBar()->minimum();
		//int const hval = tbl->widget().horizontalScrollBar()->value();
		//int const hmax = tbl->widget().horizontalScrollBar()->maximum();
		int const src_pgs = source->verticalScrollBar()->pageStep();
		int const src_vval = source->verticalScrollBar()->value();
		int const src_vmax = source->verticalScrollBar()->maximum();

		float const src_rng = src_vmax;
		if (this != source)
		{
			float grr = src_vval / src_rng;

			//qDebug(" tbl wh sync: pgs=%i val=%i max=%i  src_val=%3.2f new=%i", src_pgs, src_vval, src_vmax, grr, int(grr * (verticalScrollBar()->maximum() + verticalScrollBar()->pageStep())));
			verticalScrollBar()->setValue(int(grr * (verticalScrollBar()->maximum())));
		}
		else
		{
		}	
	}

	QModelIndex	BaseTable::moveCursor (CursorAction cursor_action, Qt::KeyboardModifiers modifiers)
	{
		bool const mod = modifiers & Qt::CTRL;
		if (mod)
		{
			QModelIndex const & curr_idx = selectionModel()->currentIndex();
			QModelIndex mod_idx = curr_idx;
			if (isModelProxy())
				mod_idx = m_table_view_proxy->mapToSource(curr_idx);

			unsigned long long const t = m_modelView->row_time(mod_idx.row());
			m_connection->requestTableActionSync(m_config.m_sync_group, t, cursor_action, modifiers, this);
		}
		return QTableView::moveCursor(cursor_action, modifiers);
	}

	void BaseTable::requestTableActionSync (unsigned long long t, int cursor_action, Qt::KeyboardModifiers modifiers, QTableView const * source)
	{
		if (this != source)
			findNearestTimeRow(t);
	}
}


