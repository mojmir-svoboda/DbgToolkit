#include "ganttwidget.h"
#include <QScrollBar>
#include <QSplitter>
#include <utils.h>
#include <utils_qstandarditem.h>
#include "../delegates.h"
#include "ganttview.h"
#include <label.h>
#include "frameview.h"
#include <syncwidgets.h>
#include <connection.h>

DataFrame::DataFrame (Connection * connection, FrameViewConfig & config, QString const & confname, QStringList const & path)
	: DockedData<e_data_frame>(connection, config, confname, path)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	m_widget = new FrameView(connection, 0, m_config, confname, path);
}


namespace gantt {

	GanttWidget::GanttWidget (Connection * oparent, QWidget * wparent, GanttConfig & cfg, QString const & fname, QStringList const & path)
		: QFrame(wparent), ActionAble(path)
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

	bool GanttWidget::handleAction (Action * a, bool sync)
	{
		return false;
	}

	void GanttWidget::commitCommands (E_ReceiveMode mode)
	{
		for (int i = 0, ie = m_queue.size(); i < ie; ++i)
		{
			DecodedCommand & cmd = m_queue[i];
			handleGanttCommand(cmd);
		}
		//m_src_model->commitCommands(mode);
	}

	void GanttWidget::handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
	{
		if (mode == e_RecvSync)
			m_src_model->handleCommand(cmd, mode);
		else
			m_queue.append(cmd);
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
		//m_connection->saveConfigForGantt(m_config, m_config.m_tag);

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


	void GanttWidget::loadConfig (QString const & path)
	{
		//QString const logpath = path + "/" + g_presetLogTag + "/" + m_config.m_tag;
		//m_config2.clear();
		//logs::loadConfig(m_config2, logpath);
		//filterWidget()->loadConfig(logpath);
	}

	void GanttWidget::saveConfig (QString const & path)
	{
	}


	void GanttWidget::appendFrameEnd (DecodedData & dd)
	{
		gantt::GanttView * gv = findOrCreateGanttView(dd.m_subtag);
		Connection::dataframeviews_t::iterator fv_it = findOrCreateFrameView(gv->config().m_sync_group);

		unsigned long long from, to;
		gv->appendFrameEnd(dd, from, to);
		(*fv_it)->widget().appendFrame(from, to);
	}

	Connection::dataframeviews_t::iterator GanttWidget::findOrCreateFrameView (int sync_group)
	{
		QString const tag = QString("%1").arg(sync_group);

		Connection::dataframeviews_t::iterator it = m_connection->dataWidgetFactory<e_data_gantt>(tag);

		m_dataframeviews.insert(sync_group, fv);
			//QModelIndex const item_idx = m_data_model->insertItemWithHint(name, template_config.m_show);
		return it;
	}

	bool parseCommand (DecodedCommand const & cmd, gantt::DecodedData & dd)
	{
		QString msg;
		QString tid;
		QString time;
		QString fgc;
		QString bgc;
		for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
		{
			if (cmd.tvs[i].m_tag == tlv::tag_msg)
				msg = cmd.tvs[i].m_val;
			else if (cmd.tvs[i].m_tag == tlv::tag_time)
				time = cmd.tvs[i].m_val;
			else if (cmd.tvs[i].m_tag == tlv::tag_tid)
				tid = cmd.tvs[i].m_val;
			else if (cmd.tvs[i].m_tag == tlv::tag_fgc)
				fgc = cmd.tvs[i].m_val;
			else if (cmd.tvs[i].m_tag == tlv::tag_bgc)
				bgc = cmd.tvs[i].m_val;
		}

		QString subtag = msg;
		int const slash_pos0 = subtag.lastIndexOf(QChar('/'));
		subtag.chop(msg.size() - slash_pos0);

		QString tag = subtag;
		int const slash_pos1 = tag.lastIndexOf(QChar('/'));
		tag.chop(tag.size() - slash_pos1);

		subtag.remove(0, slash_pos1 + 1);
		msg.remove(0, slash_pos0 + 1);

		//if (!subtag.contains("Dude"))
		//	return false;

		dd.m_time = time.toULongLong();
		dd.m_ctx = tid.toULongLong();
		dd.m_tag = tag;
		dd.m_subtag = subtag;
		dd.m_text = msg;
		return true;
	}

	bool GanttWidget::handleGanttBgnCommand (DecodedCommand const & cmd)
	{
		if (m_main_window->ganttState() == e_FtrDisabled)
			return true;

		gantt::DecodedData dd;
		if (!parseCommand(cmd, dd))
			return true;
		dd.m_type = gantt::e_GanttBgn;

		//qDebug("+decoded Gantt type=%i tag='%s' subtag='%s' text='%s'", dd.m_type, dd.m_tag.toStdString().c_str(), dd.m_subtag.toStdString().c_str(), dd.m_text.toStdString().c_str());
		appendGantt(dd);
		return true;
	}

	bool GanttWidget::handleGanttEndCommand (DecodedCommand const & cmd)
	{
		if (m_main_window->ganttState() == e_FtrDisabled)
			return true;

		gantt::DecodedData dd;
		if (!parseCommand(cmd, dd))
			return true;
		dd.m_type = gantt::e_GanttEnd;
		//qDebug("+decoded Gantt type=%i tag='%s' subtag='%s' text='%s'", dd.m_type, dd.m_tag.toStdString().c_str(), dd.m_subtag.toStdString().c_str(), dd.m_text.toStdString().c_str());
		appendGantt(dd);
		return true;
	}
	bool GanttWidget::handleGanttFrameBgnCommand (DecodedCommand const & cmd)
	{
		if (m_main_window->ganttState() == e_FtrDisabled)
			return true;

		gantt::DecodedData dd;
		if (!parseCommand(cmd, dd))
			return true;
		dd.m_type = gantt::e_GanttFrameBgn;
		appendGantt(dd);
		return true;

	}
	bool GanttWidget::handleGanttFrameEndCommand (DecodedCommand const & cmd)
	{
		if (m_main_window->ganttState() == e_FtrDisabled)
			return true;

		gantt::DecodedData dd;
		if (!parseCommand(cmd, dd))
			return true;
		dd.m_type = gantt::e_GanttFrameEnd;

		appendGantt(dd);
		//appendFrameEnd(dd);
		return true;
	}



}


