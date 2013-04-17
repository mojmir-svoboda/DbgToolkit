#include "basegantt.h"
#include <QScrollBar>
#include "connection.h"
#include "utils.h"
#include "utils_qstandarditem.h"
#include "delegates.h"

namespace gantt {

	BaseGantt::BaseGantt (Connection * oparent, QWidget * wparent, GanttConfig & cfg, QString const & fname)
		: QFrame(wparent)
		, m_config(cfg)
		, m_config_ui(cfg, this)
		, m_fname(fname)
		, m_connection(oparent)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		setConfigValuesToUI(m_config);
		onApplyButton();
		//setUpdatesEnabled(true);
		//horizontalHeader()->setSectionsMovable(true);
		//setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	}

	BaseGantt::~BaseGantt ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
	}

	void BaseGantt::onShow ()
	{
		show();
	}

	void BaseGantt::onHide ()
	{
		hide();
	}

	void BaseGantt::applyConfig (GanttConfig & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsGantt * ui = m_config_ui.ui();

	}

	void BaseGantt::onHideContextMenu ()
	{
		Ui::SettingsGantt * ui = m_config_ui.ui();
		disconnect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		disconnect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		m_config_ui.onHideContextMenu();
	}

	void BaseGantt::onShowContextMenu (QPoint const & pos)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		QRect widgetRect = geometry();
		m_config_ui.onShowContextMenu(QCursor::pos());
		Ui::SettingsGantt * ui = m_config_ui.ui();

		setConfigValuesToUI(m_config);
		connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
	}

	void BaseGantt::filteringStateChanged (int state)
	{
		if (m_config.m_hide_empty ^ state)
		{
			setUIValuesToConfig(m_config);
			//m_config.m_hide_empty = state;
			applyConfig(m_config);
			setConfigValuesToUI(m_config);
		}
	}

	void BaseGantt::setConfigValuesToUI (GanttConfig const & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsGantt * ui = m_config_ui.ui();
		
		ui->ganttShowCheckBox->blockSignals(true);
		ui->ganttShowCheckBox->setCheckState(cfg.m_show ? Qt::Checked : Qt::Unchecked);
		ui->ganttShowCheckBox->blockSignals(false);

		/*ui->filteringCheckBox->blockSignals(true);
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
		}*/
	}

	void BaseGantt::setUIValuesToConfig (GanttConfig & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsGantt * ui = m_config_ui.ui();
		m_config.m_hide_empty = ui->filteringCheckBox->checkState() == Qt::Checked;
		m_config.m_auto_scroll = ui->autoScrollCheckBox->checkState() == Qt::Checked;
		m_config.m_sync_group = ui->syncGroupSpinBox->value();
	}

	void BaseGantt::onApplyButton ()
	{
		setUIValuesToConfig(m_config);
		applyConfig(m_config);
	}

	void BaseGantt::onSaveButton ()
	{
		/*m_config.m_hsize.clear();
		m_config.m_hsize.resize(m_modelView->columnCount());
		for (int i = 0, ie = m_modelView->columnCount(); i < ie; ++i)
			m_config.m_hsize[i] = horizontalHeader()->sectionSize(i);*/
		m_connection->saveConfigForGantt(m_config, m_config.m_tag);

	}
	void BaseGantt::onResetButton () { setConfigValuesToUI(m_config); }
	void BaseGantt::onDefaultButton ()
	{
		GanttConfig defaults;
		//defaults.partialLoadFrom(m_config);
		setConfigValuesToUI(defaults);
	}

	void BaseGantt::appendGanttXY (QString const & msg)
	{
	}

	/*void BaseGantt::scrollTo (QModelIndex const & index, ScrollHint hint)
	{
		//QTableView::scrollTo(index, hint);
	}

	void BaseGantt::findNearestTimeRow (unsigned long long t)
	{
	}

	void BaseGantt::wheelEvent (QWheelEvent * event)
	{
	}

	void BaseGantt::requestGanttWheelEventSync (QWheelEvent * ev, QGanttView const * source)
	{
	}

	void BaseGantt::requestGanttActionSync (unsigned long long t, int cursor_action, Qt::KeyboardModifiers modifiers, QGanttView const * source)
	{
		if (this != source)
			findNearestTimeRow(t);
	}*/
}


