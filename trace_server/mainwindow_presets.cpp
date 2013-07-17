#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settings.h"
#include "ui_help.h"
#include "connection.h"
#include "dock.h"
#include <QMessageBox>
#include <QInputDialog>
#include "settings.h"
#include "constants.h"
#include "utils.h"

QString MainWindow::getCurrentPresetName () const
{
	QString txt = ui->presetComboBox->currentText();
	return txt;
}

QString MainWindow::promptAndCreatePresetName (QString const & app_name)
{
	if (Connection * conn = findCurrentConnection())
	{
		bool ok = false;

		QString const filled_text = getPresetPath(app_name, g_defaultPresetName);
		QStringList items(m_config.m_preset_names);
		items.push_front(filled_text);

		QString preset_name = QInputDialog::getItem(this, tr("Save current preset"), tr("Preset name:"), items, 0, true, &ok);
		if (ok && !preset_name.isEmpty())
		{
			return preset_name;
		}
	}
	return "unknown";
}

QString MainWindow::matchClosestPresetName (QString const & app_name)
{
	QString const saved_preset = getCurrentPresetName();
	QString preset_appname = saved_preset;
	int const slash_pos = preset_appname.lastIndexOf(QChar('/'));
	if (slash_pos != -1)
		preset_appname.chop(preset_appname.size() - slash_pos);
	qDebug("match preset name: curr=%s app_name=%s", preset_appname.toStdString().c_str(), app_name.toStdString().c_str());
	if (preset_appname.contains(app_name))
	{
		qDebug("got correct preset name appname/.* from combobox");
		return saved_preset;
	}
	else
	{
		qDebug("got nonmatching preset name appname/.* from combobox, loading default");
		QString const pname = getPresetPath(app_name, g_defaultPresetName);
		return pname;
	}
}

QString MainWindow::getValidCurrentPresetName ()
{
	QString txt = getCurrentPresetName();
	if (0 == txt.size())
	{
		if (Connection * conn = findCurrentConnection())
			txt = promptAndCreatePresetName(conn->getAppName());
	}
	if (0 == txt.size())
		if (Connection * conn = findCurrentConnection())
			txt = getPresetPath(conn->getAppName(), g_defaultPresetName);
	return txt;
}

void MainWindow::onSaveCurrentState ()
{
	QString const txt = getValidCurrentPresetName();
	if (txt.size())
		onSaveCurrentStateTo(txt);
}

void MainWindow::setPresetNameIntoComboBox (QString const & pname)
{
	ui->presetComboBox->setCurrentIndex(ui->presetComboBox->findText(pname));
}
void MainWindow::reloadPresetList ()
{
	ui->presetComboBox->clear();
	for (int i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
		ui->presetComboBox->addItem(m_config.m_preset_names.at(i));
}

void MainWindow::onSaveCurrentStateTo (QString const & preset_name)
{
	qDebug("%s", __FUNCTION__);
	if (preset_name.isEmpty()) return;
	//if (!getTabTrace()->currentWidget()) return;
	if (Connection * conn = findCurrentConnection())
	{
		int idx = findPresetName(preset_name);
		if (idx == -1)
			idx = addPresetName(preset_name);
		// @TODO: validate preset_name
		createPresetPath(m_config.m_appdir, preset_name);

		qDebug("new preset_name[%i]=%s", idx, preset_name.toStdString().c_str());
		
		QString const path = getPresetPath(getConfig().m_appdir, preset_name);
		conn->saveConfigs(path);
		saveLayout(preset_name);
		
		reloadPresetList ();
		setPresetNameIntoComboBox(preset_name);
		storePresetNames();
	}
	storeState();
}


void MainWindow::onAddCurrentState ()
{
	qDebug("%s", __FUNCTION__);
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = findCurrentConnection();
	if (!conn) return;

	QString const filled_text = getPresetPath(conn->getAppName(), g_defaultPresetName);
	QStringList items(m_config.m_preset_names);
	items.push_front(filled_text);

	bool ok = false;
	QString preset_name = QInputDialog::getItem(this, tr("Save current preset"), tr("Preset name:"), items, 0, true, &ok);
	if (ok && !preset_name.isEmpty())
	{
		onSaveCurrentStateTo(preset_name);
	}
}

void MainWindow::onRmCurrentState ()
{
	qDebug("%s", __FUNCTION__);
	QString const preset_name = ui->presetComboBox->currentText();
	int idx = findPresetName(preset_name);
	if (idx == -1)
		return;

	qDebug("removing preset_name[%i]=%s", idx, preset_name.toStdString().c_str());
	
	QString fname = getPresetPath(m_config.m_appdir, preset_name);
	qDebug("confirm to remove session file=%s", fname.toStdString().c_str());

	QMessageBox msg_box;
	QPushButton * b_del = msg_box.addButton(tr("Yes, Delete"), QMessageBox::ActionRole);
	QPushButton * b_abort = msg_box.addButton(QMessageBox::Abort);
	msg_box.exec();
	if (msg_box.clickedButton() == b_abort)
		return;

	QFile qf(fname);
	qf.remove();

	ui->presetComboBox->clear();

	m_config.m_preset_names.erase(std::remove(m_config.m_preset_names.begin(), m_config.m_preset_names.end(), preset_name), m_config.m_preset_names.end());
	for (int i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
		ui->presetComboBox->addItem(m_config.m_preset_names.at(i));
	//setPresetNameIntoComboBox(preset_name);
	storePresetNames();
}



void MainWindow::onPresetActivate (int idx)
{
	qDebug("%s", __FUNCTION__);
	if (idx == -1) return;
	if (Connection * conn = findCurrentConnection())
	{
		QString const & preset_name = m_config.m_preset_names.at(idx);
		onPresetActivate(conn, preset_name);
	}
}
void MainWindow::onPresetActivate () { onPresetActivate(ui->presetComboBox->currentIndex()); }
void MainWindow::onPresetActivate (QString const & pname) { onPresetActivate(ui->presetComboBox->findText(pname)); }
void MainWindow::onPresetActivate (Connection * conn, QString const & preset_name)
{
	//@TODO: toto prijde jinam
	qDebug("%s", __FUNCTION__);
	if (!conn) return;

	bool const neco = 0;
	if (neco)
	{
		conn->loadConfigs(preset_name);
		loadLayout(preset_name);

		setPresetNameIntoComboBox(preset_name);
	}
	else
	{
		ui->presetComboBox->removeItem(ui->presetComboBox->findText(preset_name));
		m_config.m_preset_names.removeAll(preset_name);
		storePresetNames();
	}
}


void MainWindow::onPresetChanged (int idx)
{
/*	qDebug("%s", __FUNCTION__);
	if (idx == -1) return;
	if (Connection * conn = findCurrentConnection())
	{
		conn->onClearCurrentFileFilter();
		QString const & preset_name = m_config.m_preset_names.at(idx);
		onPresetActivate(conn, preset_name);
	}*/
}

void MainWindow::saveLayout (QString const & preset_name)
{
	QString fname = getPresetPath(m_config.m_appdir, preset_name) + "/" + g_presetLayoutName;
	QFile file(fname);
	if (!file.open(QFile::WriteOnly))
	{
		QString msg = tr("Failed to open %1\n%2").arg(fname).arg(file.errorString());
		QMessageBox::warning(this, tr("Error"), msg);
		return;
	}

	QByteArray geo_data = saveGeometry();
	QByteArray layout_data = saveState();

	bool ok = file.putChar(static_cast<uchar>(geo_data.size()));
	if (ok)
		ok = file.write(geo_data) == geo_data.size();
	if (ok)
		ok = file.write(layout_data) == layout_data.size();

	if (!ok)
	{
		QString msg = tr("Error writing to %1\n%2").arg(fname).arg(file.errorString());
		QMessageBox::warning(this, tr("Error"), msg);
		return;
	}
}

void MainWindow::loadLayout (QString const & preset_name)
{
	QString fname = getPresetPath(m_config.m_appdir, preset_name) + "/" + g_presetLayoutName;
	QFile file(fname);
	if (!file.open(QFile::ReadOnly))
	{
		//QString msg = tr("Failed to open %1\n%2").arg(fname).arg(file.errorString());
		//QMessageBox::warning(this, tr("Error"), msg);
		return;
	}

	uchar geo_size;
	QByteArray geo_data;
	QByteArray layout_data;

	bool ok = file.getChar(reinterpret_cast<char *>(&geo_size));
	if (ok) {
		geo_data = file.read(geo_size);
		ok = geo_data.size() == geo_size;
	}
	if (ok) {
		layout_data = file.readAll();
		ok = layout_data.size() > 0;
	}

	if (ok)
		ok = restoreGeometry(geo_data);
	if (ok)
		ok = restoreState(layout_data);

	if (!ok) {
		QString msg = tr("Error reading %1").arg(fname);
		QMessageBox::warning(this, tr("Error"), msg);
		return;
	}
}



/*
void MainWindow::saveSession (SessionState const & s, QString const & preset_name) const
{
	qDebug("%s", __FUNCTION__);
	QString fname = getPresetFileName(m_config.m_appdir, preset_name);
	qDebug("store file=%s", fname.toStdString().c_str());
	saveSessionState(s, fname.toLatin1());
}

void MainWindow::storePresets ()
{
	qDebug("%s", __FUNCTION__);
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = findCurrentConnection();
	if (!conn) return;

	storePresetNames();

	for (int i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
		if (!m_config.m_preset_names.at(i).isEmpty())
			saveSession(conn->sessionState(), m_config.m_preset_names.at(i));
}

void MainWindow::saveCurrentSession (QString const & preset_name)
{
	qDebug("%s name=%s", __FUNCTION__, preset_name.toStdString().c_str());
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = findCurrentConnection();
	if (!conn) return;

	saveSession(conn->sessionState(), preset_name);
}
*/

/*bool MainWindow::loadSession (SessionState & s, QString const & preset_name)
{
	qDebug("%s name=%s", __FUNCTION__, preset_name.toStdString().c_str());
	QString fname = getPresetFileName(m_config.m_appdir, preset_name);
	qDebug("load file=%s", fname.toStdString().c_str());
	s.m_file_filters.clear();
	return loadSessionState(s, fname.toLatin1());
}
*/

