#include "mainwindow.h"
#include "connection.h"
#include <utils/utils_history.h>
#include <utils/utils.h>
#include <ui_controlbarcommon.h>
#include <ui_settings.h>
#include <QMessageBox>
#include "widgets/mixer.h"
#include <widgets/controlbar/controlbardockmanager.h>
#include <ui_controlbardockmanager.h>

void MainWindow::onMixerChanged (MixerConfig const & config)
{
// 	m_config.m_mixer = config;
// 	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
// 		(*it)->onMixerChanged(config);
}

void MainWindow::onBufferingStateChanged (int state)
{
	m_config.m_buffered = state == Qt::Checked ? true : false;
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->onBufferingStateChanged(state);
}

void MainWindow::onPresetChanged (int idx)
{
	m_config.m_preset_history.m_current_item = idx;
	m_config.saveHistory(m_appdir);
}

void MainWindow::onPresetApply ()
{
	QString const preset = getCurrentPresetName();
	onPresetApply(preset);
}

void MainWindow::onPresetSave ()
{
	QString const txt = getCurrentPresetName();
    onPresetSave(txt);
}

void MainWindow::onPresetAdd ()
{
	qDebug("%s", __FUNCTION__);
	QString const preset_name = promptAndCreatePresetName();
	onPresetSave(preset_name);
}

void MainWindow::onPresetRm ()
{
	qDebug("%s", __FUNCTION__);
	QString const preset_name = m_dock_mgr.controlUI()->presetComboBox->currentText();

	if (preset_name.isEmpty())
		return;
	qDebug("removing preset_name=%s", preset_name.toStdString().c_str());

	QString const fname = mkPresetPath(m_config.m_appdir, preset_name);
	qDebug("confirm to remove session file=%s", fname.toStdString().c_str());
	QMessageBox msg_box;
	QPushButton * b_del = msg_box.addButton(tr("Yes, Delete"), QMessageBox::ActionRole);
	QPushButton * b_abort = msg_box.addButton(QMessageBox::Abort);
	msg_box.exec();
	if (msg_box.clickedButton() == b_abort)
		return;

	QFile qf(fname);
	qf.remove();

	removeStringFromHistory(preset_name, m_dock_mgr.controlUI()->presetComboBox, m_config.m_preset_history);
	m_config.saveHistory(m_appdir);
	m_dock_mgr.controlUI()->presetComboBox->setCurrentIndex(-1);
}

void MainWindow::onPresetReset ()
{ }

void performTimeReset ();
void MainWindow::onClearAllData ()
{
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->clearAllData();

	performTimeReset();
}
void MainWindow::onColorAllLastLine ()
{
	Action a;
	a.m_type = e_ColorTagLastLine;
	a.m_src_path = dockManager().path();
	a.m_src = &m_dock_mgr;
	//a.m_dst_path = dst_path;

	for (auto it = m_dock_mgr.m_actionables.begin(), ite = m_dock_mgr.m_actionables.end(); it != ite; ++it)
		(*it)->handleAction(&a, e_Sync);
}

void MainWindow::onAllScrollToLast ()
{
	Action a;
	a.m_type = e_ScrollToLastLine;
	a.m_src_path = dockManager().path();
	a.m_src = &m_dock_mgr;
	//a.m_dst_path = dst_path;

	for (auto it = m_dock_mgr.m_actionables.begin(), ite = m_dock_mgr.m_actionables.end(); it != ite; ++it)
		(*it)->handleAction(&a, e_Sync);
}


void MainWindow::onLogsStateChanged (int state)
{
	m_config.m_logs_recv_level = state;
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->setLogsUI(state);
}
void MainWindow::onPlotsStateChanged (int state)
{
	m_config.m_plots_recv_level = state;
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->setPlotsUI(state);
}
void MainWindow::onTablesStateChanged (int state)
{
	m_config.m_tables_recv_level = state;
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->setTablesUI(state);
}
void MainWindow::onGanttsStateChanged (int state)
{
	m_config.m_gantts_recv_level = state;
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->setGanttsUI(state);
}

void MainWindow::setConfigValuesToUI (GlobalConfig const & cfg)
{
	//m_dock_mgr.controlUI()->levelSpinBox->setValue(cfg.m_level);
	//m_dock_mgr.controlUI()->mixerButton
	m_dock_mgr.controlUI()->buffCheckBox->setChecked(cfg.m_buffered);
	syncHistoryToWidget(m_dock_mgr.controlUI()->presetComboBox, cfg.m_preset_history);
// 	m_dock_mgr.controlUI()->logSlider->setValue(cfg.m_logs_recv_level);
// 	m_dock_mgr.controlUI()->plotSlider->setValue(cfg.m_plots_recv_level);
// 	m_dock_mgr.controlUI()->tableSlider->setValue(cfg.m_tables_recv_level);
// 	m_dock_mgr.controlUI()->ganttSlider->setValue(cfg.m_gantts_recv_level);
}

void MainWindow::setUIValuesToConfig (GlobalConfig & cfg)
{
}

//bool MainWindow::onTopEnabled () const { return ui_settings->onTopCheckBox->isChecked(); }


