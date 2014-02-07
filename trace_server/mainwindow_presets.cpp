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
#include "utils_history.h"

QString MainWindow::getCurrentPresetName () const
{
	QString txt = ui->presetComboBox->currentText();
	return txt;
}

QString MainWindow::promptAndCreatePresetName (QString const & app_name)
{
	if (Connection * conn = findCurrentConnection())
	{
		QString const default_pname = getPresetPath(app_name, g_defaultPresetName);

		// pre-select default_pname (for text-replace mode)
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
    QString const multitab_preset_hint = ui->multiTabPresetComboBox->currentText();
    if (!multitab_preset_hint.isEmpty())
    {
        return app_name + "/" + multitab_preset_hint;
    }
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

	mentionStringInHistory_NoRef(str, ui->presetComboBox, m_config.m_preset_history);
	m_config.saveHistory();
}

void MainWindow::mentionInMultiTabPresetHistory (QString const & str)
{
	if (str.isEmpty() || !validatePresetName(str))
		return;

	mentionStringInHistory_NoRef(str, ui->multiTabPresetComboBox, m_config.m_multitab_preset_history);
	m_config.saveHistory();
}
void MainWindow::setPresetAsCurrent (QString const & pname)
{
	ui->presetComboBox->setCurrentIndex(ui->presetComboBox->findText(pname));
}

void MainWindow::onSaveCurrentStateTo (QString const & preset_name)
{
	qDebug("%s %s", __FUNCTION__, preset_name.toStdString().c_str());
	if (Connection * conn = findCurrentConnection())
	{
		if (!validatePresetName(preset_name))
		{
			return;
		}
		createPresetPath(m_config.m_appdir, preset_name);
		conn->m_curr_preset = preset_name;
		qDebug("save to preset_name=%s", preset_name.toStdString().c_str());

		mentionStringInHistory_Ref(preset_name, ui->presetComboBox, m_config.m_preset_history);
		m_config.saveHistory();
		setPresetAsCurrent(preset_name);
		
		QString const path = getPresetPath(getConfig().m_appdir, preset_name);
		conn->saveConfigs(path);
		saveLayout(preset_name);
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

void MainWindow::onRmCurrentPreset ()
{
	qDebug("%s", __FUNCTION__);
	QString const preset_name = ui->presetComboBox->currentText();

	if (preset_name.isEmpty())
		return;

	qDebug("removing preset_name[%i]=%s", preset_name.toStdString().c_str());
	
	QString const fname = getPresetPath(m_config.m_appdir, preset_name);
	qDebug("confirm to remove session file=%s", fname.toStdString().c_str());

	QMessageBox msg_box;
	QPushButton * b_del = msg_box.addButton(tr("Yes, Delete"), QMessageBox::ActionRole);
	QPushButton * b_abort = msg_box.addButton(QMessageBox::Abort);
	msg_box.exec();
	if (msg_box.clickedButton() == b_abort)
		return;

	QFile qf(fname);
	qf.remove();

	removeStringFromHistory(preset_name, ui->presetComboBox, m_config.m_preset_history);
	m_config.saveHistory();
	ui->presetComboBox->setCurrentIndex(-1);
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
		conn->m_curr_preset = preset_name;

		mentionStringInHistory_Ref(preset_name, ui->presetComboBox, m_config.m_preset_history);
        QStringList list1 = preset_name.split("/");
		mentionStringInHistory_Ref(list1.at(1), ui->multiTabPresetComboBox, m_config.m_multitab_preset_history);
		m_config.saveHistory();
		setPresetAsCurrent(preset_name);

		QString const path = getPresetPath(m_config.m_appdir, preset_name);
		conn->loadConfigs(path);
		conn->applyConfigs();
		loadLayout(preset_name);
	}
	else
	{
		removeStringFromHistory(preset_name, ui->presetComboBox, m_config.m_preset_history);
		m_config.saveHistory();
		ui->presetComboBox->setCurrentIndex(-1);
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


