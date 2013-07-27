#include "mainwindow.h"
#include <QSettings>
#include "ui_mainwindow.h"
#include "utils_qsettings.h"

int MainWindow::findRegistryPresetName (QString const & name)
{
	for (int i = 0, ie = m_config.m_registry_pnames.size(); i < ie; ++i)
		if (m_config.m_registry_pnames[i] == name)
			return i;
	return e_InvalidItem;
}
int MainWindow::addRegistryPresetName (QString const & name)
{
	m_config.m_registry_pnames.push_back(name);
	return static_cast<int>(m_config.m_registry_pnames.size()) - 1;
}

void MainWindow::loadPresets ()
{
/*	qDebug("%s", __FUNCTION__);
	m_config.m_preset_names.clear();

	QSettings settings("MojoMir", "TraceServer");
	read_list_of_strings(settings, "known-presets", "preset", m_config.m_preset_names);
	for (int  i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
	{
		ui->presetComboBox->addItem(m_config.m_preset_names.at(i));
	}

	for (int  i = 0, ie = m_config.m_preset_names.size(); i < ie; ++i)
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
		}
	}*/
}

void MainWindow::storePresetNames ()
{
	qDebug("%s", __FUNCTION__);
	QSettings settings("MojoMir", "TraceServer");
	write_list_of_strings(settings, "known-presets", "preset", m_config.m_registry_pnames);
}

