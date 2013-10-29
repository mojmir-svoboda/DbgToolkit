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

void MainWindow::storePresetNames ()
{
	qDebug("%s", __FUNCTION__);
	QSettings settings("MojoMir", "TraceServer");
	write_list_of_strings(settings, "known-presets", "preset", m_config.m_registry_pnames);
}

