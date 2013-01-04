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
#include "serialization.h"

QString MainWindow::getCurrentPresetName () const
{
	QString txt = ui->presetComboBox->currentText();
	return txt;
}

QString MainWindow::promptAndCreatePresetName ()
{
	if (Connection * conn = m_server->findCurrentConnection())
	{
		bool ok = false;

		QString const filled_text = getPresetPath(conn->sessionState().getAppName(), g_defaultPresetName);
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

QString MainWindow::getValidCurrentPresetName ()
{
	QString txt = getCurrentPresetName();
	if (0 == txt.size())
		txt = promptAndCreatePresetName();
	if (0 == txt.size())
		if (Connection * conn = m_server->findCurrentConnection())
			txt = getPresetPath(conn->sessionState().getAppName(), g_defaultPresetName);
	return txt;
}

void MainWindow::onSaveCurrentState ()
{
	QString txt = getCurrentPresetName();
	if (0 == txt.size())
		if (Connection * conn = m_server->findCurrentConnection())
			txt = getPresetPath(conn->sessionState().getAppName(), g_defaultPresetName);
	onSaveCurrentStateTo(txt);
}

void MainWindow::onSaveCurrentStateTo (QString const & preset_name)
{
	qDebug("%s", __FUNCTION__);
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	if (!preset_name.isEmpty())
	{
		int idx = findPresetName(preset_name);
		if (idx == -1)
			idx = addPresetName(preset_name);
		createPresetPath(m_config.m_appdir, preset_name);

		qDebug("new preset_name[%i]=%s", idx, preset_name.toStdString().c_str());
		saveCurrentSession(preset_name);
		saveLayout(preset_name);

		conn->saveConfigForTables(preset_name);
		conn->saveConfigForPlots(preset_name);

		ui->presetComboBox->clear();
		for (size_t i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
			ui->presetComboBox->addItem(m_config.m_preset_names.at(i));
		setPresetNameIntoComboBox(preset_name);
		storePresetNames();
	}
}

void MainWindow::setPresetNameIntoComboBox (QString const & pname)
{
	ui->presetComboBox->setCurrentIndex(ui->presetComboBox->findText(pname));
}

void MainWindow::onAddCurrentState ()
{
	qDebug("%s", __FUNCTION__);
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QString const filled_text = getPresetPath(conn->sessionState().getAppName(), g_defaultPresetName);
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
	
	QString fname = getPresetFileName(m_config.m_appdir, preset_name);
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
	for (size_t i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
		ui->presetComboBox->addItem(m_config.m_preset_names.at(i));
	//setPresetNameIntoComboBox(preset_name);
	storePresetNames();
}


void MainWindow::onPresetActivate (QString const & pname)
{
	qDebug("%s", __FUNCTION__);
	onPresetActivate(ui->presetComboBox->findText(pname));
}

void MainWindow::onPresetActivate (Connection * conn, QString const & pname)
{
	qDebug("%s", __FUNCTION__);
	if (!conn) return;

	SessionState dummy;
	if (loadSession(dummy, pname))
	{
		conn->destroyModelFile();
		std::swap(conn->m_session_state.m_file_filters.root, dummy.m_file_filters.root);
		conn->setupModelFile();
		conn->m_session_state.m_filtered_regexps = dummy.m_filtered_regexps;
		conn->m_session_state.m_colorized_texts = dummy.m_colorized_texts;
		conn->m_session_state.m_lvl_filters = dummy.m_lvl_filters;
		conn->m_session_state.m_ctx_filters = dummy.m_ctx_filters;
		conn->m_curr_preset = pname;
		//@TODO: this blows under linux, i wonder why?
		//conn->m_session_state.m_filtered_regexps.swap(dummy.m_filtered_regexps);
		//conn->m_session_state.m_colorized_texts.swap(dummy.m_colorized_texts);

		getWidgetFile()->hideLinearParents();
		getWidgetFile()->syncExpandState();

		conn->onInvalidateFilter();
		setPresetNameIntoComboBox(pname);
		loadLayout(pname);
	}
	else
	{
		ui->presetComboBox->removeItem(ui->presetComboBox->findText(pname));
		m_config.m_preset_names.removeAll(pname);
		storePresetNames();
	}
}

void MainWindow::onPresetActivate (int idx)
{
	qDebug("%s", __FUNCTION__);
	if (idx == -1) return;
	if (Connection * conn = m_server->findCurrentConnection())
	{
		conn->onClearCurrentFileFilter();
		onPresetActivate(conn, m_config.m_preset_names.at(idx));
	}
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

void MainWindow::saveSession (SessionState const & s, QString const & preset_name) const
{
	qDebug("%s", __FUNCTION__);
	QString fname = getPresetFileName(m_config.m_appdir, preset_name);
	qDebug("store file=%s", fname.toStdString().c_str());
	saveSessionState(s, fname.toAscii());
}

void MainWindow::storePresets ()
{
	qDebug("%s", __FUNCTION__);
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	storePresetNames();

	for (size_t i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
		if (!m_config.m_preset_names.at(i).isEmpty())
			saveSession(conn->sessionState(), m_config.m_preset_names.at(i));
}

void MainWindow::saveCurrentSession (QString const & preset_name)
{
	qDebug("MainWindow::saveCurrentSession(), name=%s", preset_name.toStdString().c_str());
	if (!getTabTrace()->currentWidget()) return;
	Connection * conn = m_server->findCurrentConnection();
	if (!conn) return;

	QSettings settings("MojoMir", "TraceServer");

	saveSession(conn->sessionState(), preset_name);
}

bool MainWindow::loadSession (SessionState & s, QString const & preset_name)
{
	qDebug("%s", __FUNCTION__);
	QString fname = getPresetFileName(m_config.m_appdir, preset_name);
	qDebug("load file=%s", fname.toStdString().c_str());
	s.m_file_filters.clear();
	return loadSessionState(s, fname.toAscii());
}

void MainWindow::loadPresets ()
{
	qDebug("%s", __FUNCTION__);
	m_config.m_preset_names.clear();

	QSettings settings("MojoMir", "TraceServer");
	read_list_of_strings(settings, "known-presets", "preset", m_config.m_preset_names);
	for (size_t i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
	{
		ui->presetComboBox->addItem(m_config.m_preset_names.at(i));
	}

	// @NOTE: this is only for smooth transition only
	for (size_t i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
	{
		qDebug("reading preset: %s", m_config.m_preset_names.at(i).toStdString().c_str());
		QString const prs_name = tr("preset_%1").arg(m_config.m_preset_names[i]);
		QStringList const split = prs_name.split("/");

		if (prs_name.isEmpty())
			continue;

		if (split.size() == 2)
		{
			qDebug("split[0]: %s", split.at(0).toStdString().c_str());
			qDebug("split[1]: %s", split.at(1).toStdString().c_str());
			if (settings.childGroups().contains(split.at(0)))
			{
				settings.beginGroup(split.at(0));
				if (settings.childGroups().contains(split.at(1)))
				{
					settings.beginGroup(split.at(1));
					typedef QList<QString>			filter_regexs_t;
					typedef QList<QString>			filter_preset_t;

					filter_preset_t m_file_filters;
					filter_preset_t m_colortext_regexs;
					filter_preset_t m_colortext_colors;
					filter_preset_t m_colortext_enabled;
					filter_preset_t m_regex_filters;
					filter_preset_t m_regex_fmode;
					filter_preset_t m_regex_enabled;

					read_list_of_strings(settings, "items", "item", m_file_filters);
					read_list_of_strings(settings, "cregexps", "item", m_colortext_regexs);
					read_list_of_strings(settings, "cregexps_colors", "item", m_colortext_colors);
					read_list_of_strings(settings, "cregexps_enabled", "item", m_colortext_enabled);
					read_list_of_strings(settings, "regexps", "item", m_regex_filters);
					read_list_of_strings(settings, "regexps_fmode", "item", m_regex_fmode);
					read_list_of_strings(settings, "regexps_enabled", "item", m_regex_enabled);

					SessionState ss;
					for (int f = 0, fe = m_file_filters.size(); f < fe; ++f)
						ss.m_file_filters.set_to_state(m_file_filters.at(f), e_Checked);

					saveSession(ss, m_config.m_preset_names.at(i));

					settings.remove("");
					settings.endGroup();
				}
				settings.endGroup();
		}
		else
		{
			if (settings.childGroups().contains(prs_name))
			{
				settings.beginGroup(prs_name);
				typedef QList<QString>			filter_regexs_t;
				typedef QList<QString>			filter_preset_t;

				filter_preset_t m_file_filters;
				filter_preset_t m_colortext_regexs;
				filter_preset_t m_colortext_colors;
				filter_preset_t m_colortext_enabled;
				filter_preset_t m_regex_filters;
				filter_preset_t m_regex_fmode;
				filter_preset_t m_regex_enabled;

				read_list_of_strings(settings, "items", "item", m_file_filters);
				read_list_of_strings(settings, "cregexps", "item", m_colortext_regexs);
				read_list_of_strings(settings, "cregexps_colors", "item", m_colortext_colors);
				read_list_of_strings(settings, "cregexps_enabled", "item", m_colortext_enabled);
				read_list_of_strings(settings, "regexps", "item", m_regex_filters);
				read_list_of_strings(settings, "regexps_fmode", "item", m_regex_fmode);
				read_list_of_strings(settings, "regexps_enabled", "item", m_regex_enabled);

				SessionState ss;
				for (int f = 0, fe = m_file_filters.size(); f < fe; ++f)
					ss.m_file_filters.set_to_state(m_file_filters.at(f), e_Checked);

				saveSession(ss, m_config.m_preset_names.at(i));

				settings.remove("");
				settings.endGroup();
			}
		}


		}
		else
		{
			// @TODO
			// check if on disk
			// and if not, clear name from combobox
		}
	}
}

void MainWindow::storePresetNames ()
{
	qDebug("%s", __FUNCTION__);
	QSettings settings("MojoMir", "TraceServer");
	write_list_of_strings(settings, "known-presets", "preset", m_config.m_preset_names);
}



