#include "connection.h"
#include <QTcpSocket>
#include <QMessageBox>
#include <utils/utils.h>
#include <utils/utils_history.h>
#include <utils/utils_boost.h>
//#include "controlbarcommon.h"
#include "mainwindow.h"
#include <ui_controlbarcommon.h>
#include <trace_proto/trace_proto.h>
#include <trace_proto/encode_config.h>
#include <widgets/mixer.h>
#include <widgets/controlbar/controlbarcommon.h>

QString Connection::getClosestPresetName ()
{
	// bit clumsy, but no time to loose
	QString preset_name;
	QString const conn_name = m_control_bar->ui->presetComboBox->currentText();
	QString const parent_name = m_main_window->getCurrentPresetName();
	if (!parent_name.isEmpty() && validatePresetName(parent_name))
		preset_name = parent_name;
	else if (!conn_name.isEmpty() && validatePresetName(conn_name))
		preset_name = conn_name;
	else
		preset_name = g_defaultPresetName;
	return preset_name;
}

E_FeatureStates Connection::getClosestFeatureState (E_DataWidgetType type) const
{
	switch (type)
	{
		case e_data_log  : return static_cast<E_FeatureStates>(m_control_bar->ui->logSlider->value());
		case e_data_plot : return static_cast<E_FeatureStates>(m_control_bar->ui->plotSlider->value());
		case e_data_table: return static_cast<E_FeatureStates>(m_control_bar->ui->tableSlider->value());
		case e_data_gantt: return static_cast<E_FeatureStates>(m_control_bar->ui->ganttSlider->value());
		case e_data_frame: return static_cast<E_FeatureStates>(m_control_bar->ui->ganttSlider->value());
		default: return e_FtrDisabled;
	}
}

void Connection::setConfigValuesToUI (ConnectionConfig const & cfg)
{
	//m_control_bar->ui->levelSpinBox->setValue(cfg.m_level);
	m_control_bar->ui->buffCheckBox->setChecked(cfg.m_buffered);
	syncHistoryToWidget(m_control_bar->ui->presetComboBox, cfg.m_preset_history);
	m_control_bar->ui->logSlider->setValue(cfg.m_logs_recv_level);
	m_control_bar->ui->plotSlider->setValue(cfg.m_plots_recv_level);
	m_control_bar->ui->tableSlider->setValue(cfg.m_tables_recv_level);
	m_control_bar->ui->ganttSlider->setValue(cfg.m_gantts_recv_level);
}

void Connection::setUIValuesToConfig (ConnectionConfig & cfg)
{
	//cfg.m_level = m_control_bar->ui->levelSpinBox->value();
	cfg.m_buffered = m_control_bar->ui->buffCheckBox->isChecked();
	//
	cfg.m_logs_recv_level = static_cast<E_FeatureStates>(m_control_bar->ui->logSlider->value());
	cfg.m_plots_recv_level = static_cast<E_FeatureStates>(m_control_bar->ui->plotSlider->value());
	cfg.m_tables_recv_level = static_cast<E_FeatureStates>(m_control_bar->ui->tableSlider->value());
	cfg.m_gantts_recv_level = static_cast<E_FeatureStates>(m_control_bar->ui->ganttSlider->value());
}

void Connection::setMixerUI (MixerConfig const & cfg) { /*m_control_bar->ui->levelSpinBox->setValue(i);*/ }
void Connection::setBufferedUI (int state) { m_control_bar->ui->buffCheckBox->setChecked(state); }
void Connection::setLogsUI (int state) { m_control_bar->ui->logSlider->setValue(state); }
void Connection::setPlotsUI (int state) { m_control_bar->ui->plotSlider->setValue(state); }
void Connection::setTablesUI (int state) { m_control_bar->ui->tableSlider->setValue(state); }
void Connection::setGanttsUI (int state) { m_control_bar->ui->ganttSlider->setValue(state); }

void Connection::onLogsStateChanged (int state)
{
	m_config.m_logs_recv_level = state;
}
void Connection::onPlotsStateChanged (int state)
{
	m_config.m_plots_recv_level = state;
}
void Connection::onTablesStateChanged (int state)
{
	m_config.m_tables_recv_level = state;
}
void Connection::onGanttsStateChanged (int state)
{
	m_config.m_gantts_recv_level = state;
}


void Connection::onMixerChanged (MixerConfig const & config)
{
	m_config.m_mixer = config;
	sendConfigCommand(m_config);
}

void Connection::onMixerButton ()
{
	if (m_control_bar->ui->mixerButton->isChecked())
	{
		m_mixer->applyConfig(m_config.m_mixer);
		QPoint pt = m_control_bar->ui->mixerButton->pos();
		pt.setX(pt.x() + 32);
		//m_mixer->move(pt);
		m_mixer->move(m_control_bar->ui->mixerButton->mapToGlobal(pt));
		m_mixer->show();
	}
	else
	{
		m_mixer->hide();
	}
}

void Connection::onMixerClosed ()
{
	m_control_bar->ui->mixerButton->setChecked(false);
}

void Connection::onBufferingStateChanged (int state)
{
	m_config.m_buffered = state;
	sendConfigCommand(m_config);
}

struct ClearAllData
{
	ClearAllData () { }

	template <class T>
	void operator() (T const & t)
	{
		typedef typename T::const_iterator it_t;
		for (it_t it = t.begin(), ite = t.end(); it != ite; ++it)
			(*it)->clearAllData();
	}
};

void Connection::clearAllData()
{
	recurse(m_data_widgets, ClearAllData());
}
void Connection::onClearAllData()
{
	clearAllData();
}

void Connection::onPresetChanged (int idx)
{
	m_config.m_preset_history.m_current_item = idx;
	QString const preset_name = getCurrentPresetName();
	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);
	m_config.saveHistory(path);
}

void Connection::onPresetApply ()
{
	QString const txt = getCurrentPresetName();
	onPresetApply(txt);
}

void Connection::onPresetSave ()
{
	QString const txt = getCurrentPresetName();
	onPresetSave(txt);
}

void Connection::onPresetAdd ()
{
	qDebug("%s", __FUNCTION__);
	QString const preset_name = promptAndCreatePresetName();
	onPresetSave(preset_name);
}

void Connection::onPresetRm ()
{
	qDebug("%s", __FUNCTION__);
	QString const preset_name = m_control_bar->ui->presetComboBox->currentText();

	if (preset_name.isEmpty())
		return;

	qDebug("removing preset_name=%s", preset_name.toStdString().c_str());

	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);
	qDebug("confirm to remove session file=%s", path.toStdString().c_str());

	QMessageBox msg_box;
	QPushButton * b_del = msg_box.addButton(tr("Yes, Delete"), QMessageBox::ActionRole);
	QPushButton * b_abort = msg_box.addButton(QMessageBox::Abort);
	msg_box.exec();
	if (msg_box.clickedButton() == b_abort)
		return;

	QFile qf(path);
	qf.remove();

	removeStringFromHistory(preset_name, m_control_bar->ui->presetComboBox, m_config.m_preset_history);
	m_control_bar->ui->presetComboBox->setCurrentIndex(-1);
}

void Connection::onPresetReset ()
{
}

//////////////////////////////////////////////////

QString Connection::getCurrentPresetName () const
{
	QString txt = m_control_bar->ui->presetComboBox->currentText();
	if (txt.isEmpty())
		txt = g_defaultPresetName;
	return txt;
}

void Connection::setPresetAsCurrent (QString const & pname)
{
	m_control_bar->ui->presetComboBox->setCurrentIndex(m_control_bar->ui->presetComboBox->findText(pname));
}

void Connection::onPresetApply (QString const & preset_name)
{
	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);
	mentionStringInHistory_Ref(preset_name, m_control_bar->ui->presetComboBox, m_config.m_preset_history);
	m_config.saveHistory(path);

	if (checkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name))
	{
		m_curr_preset = preset_name;

		loadConfigs(path);
		applyConfigs();
		m_main_window->loadLayout(path + "/" + g_presetLayoutName);
	}
	/*else
	{
		removeStringFromHistory(preset_name, m_control_bar->ui->presetComboBox, m_config.m_preset_history);
		m_config.saveHistory(path);
		m_control_bar->ui->presetComboBox->setCurrentIndex(-1);
	}*/
	setPresetAsCurrent(preset_name);
}

void Connection::onPresetSave (QString const & preset_name)
{
	qDebug("%s %s", __FUNCTION__, preset_name.toStdString().c_str());
	if (!validatePresetName(preset_name))
		return;

	qDebug("SaveAs to preset_name=%s", preset_name.toStdString().c_str());
	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);
	mentionStringInHistory_Ref(preset_name, m_control_bar->ui->presetComboBox, m_config.m_preset_history);
	m_config.saveHistory(path);
	setPresetAsCurrent(preset_name);

	createAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);

	m_curr_preset = preset_name; // @TODO: ?? or only on apply preset?

	saveConfigs(path);
	m_main_window->saveLayout(path + "/" + g_presetLayoutName);
}

/*void Connection::mentionInPresetHistory (QString const & str)
{
	if (str.isEmpty() || !validatePresetName(str))
		return;

	mentionStringInHistory_NoRef(str, m_control_bar->ui->presetComboBox, m_config.m_preset_history);
	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);
	m_config.saveHistory(path);
}*/


