#include "basegantt.h"
#include <QScrollBar>
#include <QSplitter>
#include "connection.h"
#include "utils.h"
#include "utils_qstandarditem.h"
#include "delegates.h"
#include "ganttview.h"
#include "label.h"
#include "frameview.h"
#include "syncwidgets.h"


DataFrameView::DataFrameView (Connection * parent, FrameViewConfig & config, QString const & fname)
	: m_parent(parent)
	, m_wd(0)
	, m_widget(0)
	, m_config(config)
	, m_fname(fname)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	m_widget = new FrameView(parent, 0, m_config, fname);
}
DataFrameView::~DataFrameView ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	delete m_widget;
	m_widget = 0;
}
void DataFrameView::onShow ()
{
	m_wd->show();
	m_widget->onShow();
}
void DataFrameView::onHide ()
{
	//m_wd->hide();
	m_widget->onHide();
	QTimer::singleShot(0, m_wd, SLOT(hide()));
}



namespace gantt {

	GanttWidget::GanttWidget (Connection * oparent, QWidget * wparent, GanttConfig & cfg, QString const & fname)
		: QFrame(wparent)
		, m_config(cfg)
		, m_config_ui(cfg, this)
		, m_fname(fname)
		, m_connection(oparent)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		m_layout = new QSplitter(Qt::Vertical);
		m_layout->setContentsMargins(QMargins(0, 0, 0, 0));
		QGridLayout * grid = new QGridLayout(this);
		grid->setContentsMargins(QMargins(0, 0, 0, 0));
		grid->addWidget(m_layout, 0, 0);
		grid->setVerticalSpacing(0);
		grid->setHorizontalSpacing(0);
		setLayout(grid);

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
	}

	GanttWidget::~GanttWidget ()
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		for (ganttviews_t::iterator it = m_ganttviews.begin(), ite = m_ganttviews.end(); it != ite; ++it)
		{
			GanttView * ganttview = *it;
			//ganttview->m_ganttview->detach();
			delete ganttview;
		}
		m_ganttviews.clear();
		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
	}

	void GanttWidget::onShow ()
	{
		show();
	}

	void GanttWidget::onHide ()
	{
		hide();
	}

	GanttWidget::ganttviews_t::iterator GanttWidget::mkGanttView (QString const & subtag)
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
		m_layout->addWidget(gv);

		return m_ganttviews.insert(subtag, gv);
	}


	GanttView * GanttWidget::findGanttView (QString const & subtag)
	{
		ganttviews_t::const_iterator it = m_ganttviews.find(subtag);
		if (it == m_ganttviews.end())
			return 0;
		return *it;
	}
	GanttView * GanttWidget::findOrCreateGanttView (QString const & subtag)
	{
		ganttviews_t::const_iterator it = m_ganttviews.find(subtag);
		if (it == m_ganttviews.end())
			it = mkGanttView(subtag);
		return *it;
	}

	void GanttWidget::onHideContextMenu ()
	{
		Ui::SettingsGantt * ui = m_config_ui.ui();
		disconnect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		disconnect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		m_config_ui.onHideContextMenu();
	}

	void GanttWidget::onShowContextMenu (QPoint const & pos)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		m_config_ui.onShowContextMenu(QCursor::pos());
		Ui::SettingsGantt * ui = m_config_ui.ui();

		setConfigValuesToUI(m_config);
		connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		connect(ui->ganttViewComboBox, SIGNAL(activated(int)), this, SLOT(onGanttViewActivate(int)));
		connect(ui->fitSelectionButton, SIGNAL(clicked()), this, SLOT(onFitSelectionButton()));
		connect(ui->fitAllButton, SIGNAL(clicked()), this, SLOT(onFitAllButton()));
		connect(ui->fitFrameButton, SIGNAL(clicked()), this, SLOT(onFitFrameButton()));
		connect(ui->prevFrameButton, SIGNAL(clicked()), this, SLOT(onPrevFrameButton()));
		connect(ui->nextFrameButton, SIGNAL(clicked()), this, SLOT(onNextFrameButton()));
		connect(ui->nextFrameButton, SIGNAL(valueChanged(int)), this, SLOT(onFrameValueChanged(int)));
	}

	void GanttWidget::applyConfig (GanttConfig & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
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

	void GanttWidget::showGanttView (GanttView * item, bool on)
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

	void GanttWidget::setConfigValuesToUI (GanttConfig const & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
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

	void GanttWidget::setUIValuesToConfig (GanttConfig & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsGantt * ui = m_config_ui.ui();
		m_config.m_show = ui->globalShowCheckBox->checkState() == Qt::Checked;

		for (size_t i = 0, ie = cfg.m_gvcfg.size(); i < ie; ++i)
		{
			GanttViewConfig & gvcfg = cfg.m_gvcfg[i];
			setUIValuesToViewConfig(gvcfg);
		}
	}

	void GanttWidget::onApplyButton ()
	{
		setUIValuesToConfig(m_config);
		applyConfig(m_config);
	}

	void GanttWidget::onSaveButton ()
	{
		/*m_config.m_hsize.clear();
		m_config.m_hsize.resize(m_modelView->columnCount());
		for (int i = 0, ie = m_modelView->columnCount(); i < ie; ++i)
			m_config.m_hsize[i] = horizontalHeader()->sectionSize(i);*/
		m_connection->saveConfigForGantt(m_config, m_config.m_tag);

	}
	void GanttWidget::onResetButton () { setConfigValuesToUI(m_config); }
	void GanttWidget::onDefaultButton ()
	{
		GanttConfig defaults;
		//defaults.partialLoadFrom(m_config);
		setConfigValuesToUI(defaults);
	}

	void GanttWidget::setViewConfigValuesToUI (GanttViewConfig const & gvcfg)
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

	void GanttWidget::setUIValuesToViewConfig (GanttViewConfig & gvcfg)
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


	void GanttWidget::onGanttViewActivate (int idx)
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

	void GanttWidget::onClearAllDataButton ()
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

	void GanttWidget::onClearGanttViewDataButton ()
	{
	}

	void GanttWidget::onFitAllButton ()
	{
		Ui::SettingsGantt * ui = m_config_ui.ui();
		QString const & gvname = ui->ganttViewComboBox->currentText();
		for (size_t i = 0, ie = m_config.m_gvcfg.size(); i < ie; ++i)
		{
			GanttViewConfig const & gvcfg = m_config.m_gvcfg[i];
			if (gvcfg.m_tag == gvname)
			{
			}
		}
	}
	void GanttWidget::onFitFrameButton ()
	{
		Ui::SettingsGantt * ui = m_config_ui.ui();
		QString const & gvname = ui->ganttViewComboBox->currentText();
		for (size_t i = 0, ie = m_config.m_gvcfg.size(); i < ie; ++i)
		{
			GanttViewConfig const & gvcfg = m_config.m_gvcfg[i];
			if (gvcfg.m_tag == gvname)
			{
			}
		}
	}
	void GanttWidget::onPrevFrameButton ()
	{
		Ui::SettingsGantt * ui = m_config_ui.ui();
		QString const & gvname = ui->ganttViewComboBox->currentText();
		for (size_t i = 0, ie = m_config.m_gvcfg.size(); i < ie; ++i)
		{
			GanttViewConfig const & gvcfg = m_config.m_gvcfg[i];
			if (gvcfg.m_tag == gvname)
			{
			}
		}
	}
	void GanttWidget::onNextFrameButton ()
	{
		Ui::SettingsGantt * ui = m_config_ui.ui();
		QString const & gvname = ui->ganttViewComboBox->currentText();
		for (size_t i = 0, ie = m_config.m_gvcfg.size(); i < ie; ++i)
		{
			GanttViewConfig const & gvcfg = m_config.m_gvcfg[i];
			if (gvcfg.m_tag == gvname)
			{
			}
		}
	}
	void GanttWidget::onFrameValueChanged (int f)
	{
		Ui::SettingsGantt * ui = m_config_ui.ui();
		QString const & gvname = ui->ganttViewComboBox->currentText();
		for (size_t i = 0, ie = m_config.m_gvcfg.size(); i < ie; ++i)
		{
			GanttViewConfig const & gvcfg = m_config.m_gvcfg[i];
			if (gvcfg.m_tag == gvname)
			{
				qDebug("set frame %i", f);
				m_ganttviews[gvname]->gotoFrame(f);
			}
		}
	}



	void GanttWidget::performTimeSynchronization (int sync_group, unsigned long long time, void * source)
	{
		qDebug("%s syncgrp=%i time=%i", __FUNCTION__, sync_group, time);
		/*for (ganttviews_t::iterator it = m_ganttviews.begin(), ite = m_ganttviews.end(); it != ite; ++it)
		{
			GanttView * ganttview = *it;
			ganttview->gotoFrame(frame);
		}*/
	}

	void GanttWidget::performFrameSynchronization (int sync_group, unsigned long long frame, void * source)
	{
		qDebug("%s syncgrp=%i frame=%i", __FUNCTION__, sync_group, frame);
		for (ganttviews_t::iterator it = m_ganttviews.begin(), ite = m_ganttviews.end(); it != ite; ++it)
		{
			GanttView * ganttview = *it;
			ganttview->gotoFrame(frame);
		}
	}

	/*void BaseGantt::requestFrameSynchronization (int sync_group, unsigned long long time, void * source)
	{
	}*/

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


	



	void GanttWidget::appendFrameEnd (DecodedData & dd)
	{
		gantt::GanttView * gv = findOrCreateGanttView(dd.m_subtag);
		dataframeviews_t::iterator fv_it = findOrCreateFrameView(gv->config().m_sync_group);

		unsigned long long from, to;
		gv->appendFrameEnd(dd, from, to);
		(*fv_it)->widget().appendFrame(from, to);
	}

	dataframeviews_t::iterator GanttWidget::findOrCreateFrameView (int sync_group)
	{
		char tmp[] = "frames";
		QString const name = m_connection->sessionState().getAppName() + "/" + tmp + "/" + QString("%1").arg(sync_group);

		dataframeviews_t::iterator it = m_dataframeviews.find(sync_group);
		if (it == m_dataframeviews.end())
		{
			qDebug("gantt: creating frameview %i", name.toStdString().c_str());
			// new data gantt
			FrameViewConfig template_config;

			QString fname;
			DataFrameView * const fv = new DataFrameView(m_connection, template_config, fname);
			it = m_dataframeviews.insert(sync_group, fv);
			//QModelIndex const item_idx = m_data_model->insertItemWithHint(name, template_config.m_show);

			fv->m_wd = m_connection->getMainWindow()->dockManager().mkDockWidget(m_connection->getMainWindow(), &fv->widget(), template_config.m_show, name);
			bool const visible = template_config.m_show;
			//m_data_model->setData(item_idx, QVariant(visible ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
			if (m_connection->getMainWindow()->ganttState() == e_FtrEnabled && visible)
			{
				//m_main_window->loadLayout(preset_name);
				fv->onShow();
			}
			else
			{
				fv->onHide();
			}
		}
		return it;
	}


}


