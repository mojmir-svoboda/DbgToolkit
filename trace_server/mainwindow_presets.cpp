#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settings.h"
#include "ui_help.h"
#include "connection.h"
#include "dock.h"
#include <QMessageBox>
#include <QInputDialog>
#include "constants.h"
#include "utils.h"

QString MainWindow::getCurrentPresetName () const
{
	QString txt = ui->presetComboBox->currentText();
	return txt;
}

/*int MainWindow::presetCandidates (QString const & appname, QStringList & candidates, bool & default_present)
{
	QStringList subdirs;
	if (int const n = findPresetsForApp(m_config.m_appdir, appname, subdirs))
	{
		default_present = false;
		QStringList candidates;
		foreach (QString const & s, subdirs)
		{
			QString test_preset_name = getAppName() + "/" + s;
			QString const cfg_fname = getDataTagFileName(getConfig().m_appdir, test_preset_name, preset_prefix, tag);
			if (existsFile(cfg_fname))
			{
				if (s == QString(g_defaultPresetName))
					default_present = true;
				candidates << test_preset_name;
			}
			appendPresetNoFocus(test_preset_name);
		}
	}
	return candidates.size();
}*/

QString MainWindow::promptAndCreatePresetName (QString const & app_name)
{
	if (Connection * conn = findCurrentConnection())
	{
		QString const default_pname = getPresetPath(app_name, g_defaultPresetName);

		// pre-select default_pname (for text-replace mode)
		//
		QStringList items;
		findPresetsForApp(m_config.m_appdir, app_name, items);
		items.push_front(default_pname);

		QString preset_name;
		bool ok = true;
		while (ok)
		{
			QString const pname = QInputDialog::getItem(this, tr("Save current preset"), tr("Preset name:"), items, 0, true, &ok);
			if (ok && validatePresetName(pname))
			{
				preset_name = pname;
				break;
			}
		}
		if (!validatePresetName(preset_name))
			return default_pname;
		return preset_name;
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
		qDebug("got nonmatching preset name appname/.* from combobox");
		return QString();
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
	else
		storeState();
}

void MainWindow::mentionInPresetHistory (QString const & str)
{
	if (str.isEmpty() || !validatePresetName(str))
		return;

	m_config.m_preset_history.insert_no_refcount(str);
	m_config.saveHistory();
	syncPresetWithHistory();
	//ui->presetComboBox->setCurrentIndex(ui->presetComboBox->findText(str));
}

void MainWindow::syncPresetWithHistory ()
{
	ui->presetComboBox->clear();
	for (size_t i = 0, ie = m_config.m_preset_history.size(); i < ie; ++i)
		ui->presetComboBox->addItem(m_config.m_preset_history[i]);
}

void MainWindow::setPresetAsCurrent (QString const & pname)
{
	ui->presetComboBox->setCurrentIndex(ui->presetComboBox->findText(pname));
}

void MainWindow::onSaveCurrentStateTo (QString const & preset_name)
{
	qDebug("%s", __FUNCTION__);
	if (Connection * conn = findCurrentConnection())
	{
		if (!validatePresetName(preset_name))
		{
			return;
		}
		createPresetPath(m_config.m_appdir, preset_name);
		qDebug("save to preset_name=%s", preset_name.toStdString().c_str());
		
		QString const path = getPresetPath(getConfig().m_appdir, preset_name);
		conn->saveConfigs(path);
		saveLayout(preset_name);

		m_config.m_preset_history.insert(preset_name);
		m_config.saveHistory();
		syncPresetWithHistory();
		setPresetAsCurrent(preset_name);


		//reloadPresetList ();
		//setPresetNameIntoComboBox(preset_name);
		//storePresetNames();
	}
	storeState();
}


void MainWindow::onAddPreset ()
{
	qDebug("%s", __FUNCTION__);
	if (Connection * conn = findCurrentConnection())
	{
  		QString const preset_name = promptAndCreatePresetName(conn->getAppName());
		onSaveCurrentStateTo(preset_name);		
	}
}

void MainWindow::onRmCurrentState ()
{
	qDebug("%s", __FUNCTION__);
/*	QString const preset_name = ui->presetComboBox->currentText();
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
	storePresetNames();*/
}

void MainWindow::onPresetActivate (int idx)
{
	onPresetActivate(findCurrentConnection(), getCurrentPresetName());
}
void MainWindow::onPresetActivate ()
{
	onPresetActivate(findCurrentConnection(), getCurrentPresetName());
}
void MainWindow::onPresetActivate (Connection * conn, QString const & preset_name)
{
	//@TODO: toto prijde jinam
	qDebug("%s", __FUNCTION__);
	if (!conn) return;

	if (checkPresetPath(m_config.m_appdir, preset_name))
	{
		QString const path = getPresetPath(m_config.m_appdir, preset_name);
		conn->loadConfigs(path);
		conn->applyConfigs();
		loadLayout(preset_name);
		m_config.m_preset_history.insert(preset_name);
		syncPresetWithHistory();
		setPresetAsCurrent(preset_name);
		m_config.saveHistory();
	}
	else
	{
		m_config.m_preset_history.remove(preset_name);
		syncPresetWithHistory();
		setPresetAsCurrent(preset_name);
	}
}


void MainWindow::onPresetChanged (int idx)
{
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

