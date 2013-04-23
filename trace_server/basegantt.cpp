#include "basegantt.h"
#include <QScrollBar>
#include "connection.h"
#include "utils.h"
#include "utils_qstandarditem.h"
#include "delegates.h"
#include "ganttview.h"

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

		m_layout = new QGridLayout;
		setLayout(m_layout);

		setConfigValuesToUI(m_config);
		onApplyButton();
		//setUpdatesEnabled(true);
		//horizontalHeader()->setSectionsMovable(true);
		//setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	}

	BaseGantt::~BaseGantt ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		for (ganttviews_t::iterator it = m_ganttviews.begin(), ite = m_ganttviews.end(); it != ite; ++it)
		{
			GanttView * ganttview = *it;
			//ganttview->m_ganttview->detach();
			delete ganttview;
		}
		m_ganttviews.clear();
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

	BaseGantt::ganttviews_t::iterator BaseGantt::mkGanttView (QString const & subtag)
	{
		GanttViewConfig * c = 0;
		bool found = false;
		for (size_t i = 0, ie = m_config.m_gvcfg.size(); i < ie; ++i)
			if (m_config.m_gvcfg[i].m_tag == subtag)
			{
				c = &m_config.m_gvcfg[i];
				found = true;
			}
		if (!found)
		{
			GanttViewConfig c2;
			c2.m_tag = subtag;
			//c2.m_label = subtag;
			//c2.m_color = allocColor();
			m_config.m_gvcfg.push_back(c2);
			c = &m_config.m_gvcfg.back();
		}

		c->setTimeUnits(c->m_strtimeunits);
		GanttView * gv = new GanttView(m_connection, this, *c, subtag);

		size_t const n = m_subtags.size();
		m_subtags.push_back(subtag);
		m_layout->addWidget(gv, n, 0);
		return m_ganttviews.insert(subtag, gv);
	}


	GanttView * BaseGantt::findGanttView (QString const & subtag)
	{
		ganttviews_t::const_iterator it = m_ganttviews.find(subtag);
		if (it == m_ganttviews.end())
			return 0;
		return *it;
	}
	GanttView * BaseGantt::findOrCreateGanttView (QString const & subtag)
	{
		ganttviews_t::const_iterator it = m_ganttviews.find(subtag);
		if (it == m_ganttviews.end())
			it = mkGanttView(subtag);
		return *it;
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
		m_config_ui.onShowContextMenu(QCursor::pos());
		Ui::SettingsGantt * ui = m_config_ui.ui();

		setConfigValuesToUI(m_config);
		connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		connect(ui->ganttViewComboBox, SIGNAL(activated(int)), this, SLOT(onGanttViewActivate(int)));
	}

	void BaseGantt::applyConfig (GanttConfig & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsGantt * ui = m_config_ui.ui();

		for (size_t i = 0, ie = m_config.m_gvcfg.size(); i < ie; ++i)
		{
			GanttViewConfig & gvc = cfg.m_gvcfg[i];

			gvc.setTimeUnits(gvc.m_strtimeunits);
			if (GanttView * const gv = findOrCreateGanttView(gvc.m_tag))
			{
				showGanttView(gv, gvc.m_show);
				gv->applyConfig(gvc);
			}
		}
	}

	void BaseGantt::showGanttView (GanttView * item, bool on)
	{
		if (on)
			item->show();
		else
			item->hide();
	}

	/*void BaseGantt::filteringStateChanged (int state)
	{
		if (m_config.m_hide_empty ^ state)
		{
			setUIValuesToConfig(m_config);
			//m_config.m_hide_empty = state;
			applyConfig(m_config);
			setConfigValuesToUI(m_config);
		}
	}*/

	void BaseGantt::setConfigValuesToUI (GanttConfig const & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsGantt * ui = m_config_ui.ui();
		
		ui->globalShowCheckBox->blockSignals(true);
		ui->globalShowCheckBox->setCheckState(cfg.m_show ? Qt::Checked : Qt::Unchecked);
		ui->globalShowCheckBox->blockSignals(false);

		ui->ganttViewComboBox->clear();
		for (size_t i = 0, ie = cfg.m_gvcfg.size(); i < ie; ++i)
		{
			GanttViewConfig const & gvcfg = cfg.m_gvcfg[i];
			ui->ganttViewComboBox->addItem(gvcfg.m_tag);
		}
		if (cfg.m_gvcfg.size())
			setViewConfigValuesToUI(cfg.m_gvcfg[0]);
	}

	void BaseGantt::setUIValuesToConfig (GanttConfig & cfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsGantt * ui = m_config_ui.ui();
		m_config.m_show = ui->globalShowCheckBox->checkState() == Qt::Checked;

		for (size_t i = 0, ie = cfg.m_gvcfg.size(); i < ie; ++i)
		{
			GanttViewConfig & gvcfg = cfg.m_gvcfg[i];
			setUIValuesToViewConfig(gvcfg);
		}
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

	void BaseGantt::setViewConfigValuesToUI (GanttViewConfig const & gvcfg)
	{
		Ui::SettingsGantt * ui = m_config_ui.ui();
		ui->showGanttViewCheckBox->setCheckState(gvcfg.m_show ? Qt::Checked : Qt::Unchecked);
		ui->timeUnitsComboBox->setCurrentIndex(ui->timeUnitsComboBox->findText(gvcfg.m_strtimeunits));
		ui->scaleDoubleSpinBox->setValue(gvcfg.m_scale);
		ui->syncGroupSpinBox->setValue(gvcfg.m_sync_group);
		ui->fontSizeSpinBox->setValue(gvcfg.m_fontsize);
		ui->autoColorCheckBox->setCheckState(gvcfg.m_auto_color ? Qt::Checked : Qt::Unchecked);
		ui->yScalingCheckBox->setCheckState(gvcfg.m_y_scaling ? Qt::Checked : Qt::Unchecked);
		//gvcfg.m_curve_color->setCurrentColor(ccfg.m_color);
		//m_config_ui.m_symbol_color->setCurrentColor(ccfg.m_color);
	}

	void BaseGantt::setUIValuesToViewConfig (GanttViewConfig & gvcfg)
	{
		Ui::SettingsGantt * ui = m_config_ui.ui();
		gvcfg.m_show = ui->showGanttViewCheckBox->isChecked();
		gvcfg.m_strtimeunits = ui->timeUnitsComboBox->currentText();
		gvcfg.setTimeUnits(gvcfg.m_strtimeunits);
		gvcfg.m_scale = ui->scaleDoubleSpinBox->value();
		gvcfg.m_sync_group = ui->syncGroupSpinBox->value();
		gvcfg.m_fontsize = ui->fontSizeSpinBox->value();
		gvcfg.m_auto_color = ui->autoColorCheckBox->isChecked();
		gvcfg.m_y_scaling = ui->yScalingCheckBox->isChecked();
	}


	void BaseGantt::onGanttViewActivate (int idx)
	{
		Ui::SettingsGantt * ui = m_config_ui.ui();
		QString const & gvname = ui->ganttViewComboBox->currentText();
		for (size_t i = 0, ie = m_config.m_gvcfg.size(); i < ie; ++i)
		{
			GanttViewConfig const & gvcfg = m_config.m_gvcfg[i];
			if (gvcfg.m_tag == gvname)
			{
				setViewConfigValuesToUI(gvcfg);
				break;
			}
		}
	}

	void BaseGantt::onClearAllDataButton ()
	{
		for (ganttviews_t::iterator it = m_ganttviews.begin(), ite = m_ganttviews.end(); it != ite; ++it)
		{
			GanttView * ganttview = *it;
			//if (ganttview->m_data)
			{
				//ganttview->m_data->clear();
				//ganttview->m_ganttview->setRawSamples(0, 0, 0);
			}
		}
		//update();
	}

	void BaseGantt::onClearGanttViewDataButton ()
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


