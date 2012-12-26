#include "basetable.h"
#include <QTimer>
#include "editableheaderview.h"
#include "sparseproxy.h"
#include "connection.h"

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

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
		connect(this, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(onTableDoubleClicked(QModelIndex const &)));

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
		horizontalHeader()->setResizeMode(QHeaderView::Interactive);
		verticalHeader()->setResizeMode(QHeaderView::Interactive);

		if (!m_table_view_proxy)
		{
			m_table_view_proxy = new SparseProxyModel(this);
			m_table_view_proxy->setSourceModel(m_modelView);
		}

		verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance

		setConfigValues(m_config);
		QTimer::singleShot(0, this, SLOT(onApplyButton()));
		setUpdatesEnabled(true);
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

	void BaseTable::applyConfig (TableConfig const & pcfg)
	{
		// FIXME: for unknown reasons this fn is called multiple times (from onApplyButton)
		qDebug("%s this=0x%08x", __FUNCTION__, this);

		//killTimer(m_timer);
		//m_timer = startTimer(1000);
		setModel(m_modelView);
		qDebug("table: m_hsize.sz=%i model.sz=%i", m_config.m_hsize.size(), model()->columnCount());
		for (int i = 0, ie = m_config.m_hsize.size(); i < ie; ++i)
		{
			horizontalHeader()->blockSignals(1);
			horizontalHeader()->resizeSection(i, m_config.m_hsize.at(i));
			qDebug("table: hdr[%i]=%i", i, m_config.m_hsize.at(i));
			horizontalHeader()->blockSignals(0);
		}

		if (m_config.m_hide_empty)
		{
			qDebug("table: +++ proxy");
			static_cast<SparseProxyModel *>(m_table_view_proxy)->force_update();
			m_modelView->setProxy(m_table_view_proxy);
			setModel(m_table_view_proxy);
		}
		else
		{
			qDebug("table: --- proxy");
			setModel(m_modelView);
			m_modelView->setProxy(0);
		}
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

		setConfigValues(m_config);
		connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		//connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(onResetButton()));
		//connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(onDefaultButton()));
	}

	void BaseTable::setConfigValues (TableConfig const & pcfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();
		
		ui->hideEmptyCheckBox->setCheckState(m_config.m_hide_empty ? Qt::Checked : Qt::Unchecked);
		ui->autoScrollCheckBox->setCheckState(m_config.m_auto_scroll ? Qt::Checked : Qt::Unchecked);
		ui->syncGroupSpinBox->setValue(m_config.m_sync_group);
	}

	void BaseTable::onApplyButton ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsTable * ui = m_config_ui.ui();

		m_config.m_hide_empty = ui->hideEmptyCheckBox->checkState() == Qt::Checked;
		m_config.m_auto_scroll = ui->autoScrollCheckBox->checkState() == Qt::Checked;
		m_config.m_sync_group = ui->syncGroupSpinBox->value();

		applyConfig(m_config);
	}

	void BaseTable::onSaveButton ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);

		m_config.m_hsize.clear();
		m_config.m_hsize.resize(m_modelView->columnCount());
		qDebug("table: m_hsize.sz=%i model.sz=%i", m_config.m_hsize.size(), m_modelView->columnCount());
		for (int i = 0, ie = m_modelView->columnCount(); i < ie; ++i)
		{
			m_config.m_hsize[i] = horizontalHeader()->sectionSize(i);
		}
		saveConfig(m_config, m_fname);
	}
	void BaseTable::onResetButton () { setConfigValues(m_config); }
	void BaseTable::onDefaultButton ()
	{
		TableConfig defaults;
		//defaults.partialLoadFrom(m_config);
		setConfigValues(defaults);
	}

	void BaseTable::onSectionResized (int idx, int old_size, int new_size)
	{
		if (idx < 0) return;
		size_t const curr_sz = m_config.m_hsize.size();
		if (idx < curr_sz)
		{
			qDebug("%s this=0x%08x hsize[%i]=%i", __FUNCTION__, this, idx, new_size);
			//setColumnWidth(idx, new_size);
		}
		else
		{
			m_config.m_hsize.resize(idx + 1);
			for (size_t i = curr_sz; i < idx + 1; ++i)
				m_config.m_hsize[i] = 32;
		}
		m_config.m_hsize[idx] = new_size;
	}

	void BaseTable::appendTableXY (int x, int y, QString const & time, QString const & msg)
	{
		m_modelView->appendTableXY(x, y, time, msg);

		if (m_config.m_auto_scroll)
			scrollToBottom();

		if (m_modelView->columnCount() < m_config.m_hsize.size())
			for (int i = 0, ie = m_config.m_hsize.size(); i < ie; ++i)
			{
				if (m_config.m_hsize[i] != horizontalHeader()->sectionSize(i))
				{
					// @FIXME: tmp hack to force resize of columns
					horizontalHeader()->blockSignals(1);
					horizontalHeader()->resizeSection(i, m_config.m_hsize.at(i));
					//qDebug("table: rsz hdr[%i]=%i", i, m_config.m_hsize.at(i));
					horizontalHeader()->blockSignals(0);
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
		size_t closest_i = 0;
		int closest_dist = 1024 * 1024;
		for (size_t i = 0; i < m_modelView->rowCount(); ++i)
		{
			int const diff = m_modelView->row_time(i) - t;
			int const d = abs(diff);
			if (d < closest_dist)
			{
				closest_i = i;
				closest_dist = d;
			}
		}

		qDebug("table: nearest index= %i/%i", closest_i, m_modelView->rowCount());

		QModelIndex const curr = currentIndex();
		QModelIndex const idx = m_modelView->index(closest_i, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
		qDebug("table: findNearestTime curr=(%i, %i)  new=(%i, %i)", curr.row(), curr.column(), idx.row(), idx.column());
		if (isModelProxy())
		{
			scrollTo(m_table_view_proxy->mapFromSource(idx), QAbstractItemView::PositionAtCenter);
			selectionModel()->select(m_table_view_proxy->mapFromSource(idx), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		}
		else
		{
			scrollTo(idx, QAbstractItemView::PositionAtCenter);
			selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		}
	}

}

