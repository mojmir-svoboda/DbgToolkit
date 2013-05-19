#pragma once

template <int TypeN>
bool Connection::loadConfigFor (QString const & preset_name, typename SelectConfig<TypeN>::type & config, QString const & tag)
{
	char const * preset_prefix = g_fileTags[TypeN];
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, preset_prefix, tag);
	qDebug("loadCfgFor<>: load cfg file=%s", fname.toStdString().c_str());
	return loadConfig(config, fname);
}

template <int TypeN>
typename SelectIterator<TypeN>::type
Connection::dataWidgetFactory (QString const tag)
{
	char const * preset_prefix = g_fileTags[TypeN];
	QString const log_name = sessionState().getAppName() + "/" + preset_prefix + "/" + tag;

	typedef typename SelectIterator<TypeN>::type iterator;
	iterator it = m_data.get<TypeN>().find(tag);
	if (it == m_data.get<TypeN>().end())
	{
		qDebug("datawidget factory: creating type=%i with %s", TypeN, tag.toStdString().c_str());

		typedef typename SelectWidget<TypeN>::type widget_t;
		typedef typename SelectConfig<TypeN>::type config_t;

		config_t template_config;
		template_config.m_tag = tag;

		QString const preset_name = m_main_window->matchClosestPresetName(sessionState().getAppName());
		QString fname;
		if (!preset_name.isEmpty())
		{
			fname = getDataTagFileName(getConfig().m_appdir, preset_name, preset_prefix, tag);
			loadConfigFor<TypeN>(preset_name, template_config, tag);
		}
		
		DataLog * const dp = new DataLog(this, template_config, fname);
		it = m_data.get<TypeN>().insert(tag, dp);

		dp->m_wd = m_main_window->m_dock_mgr.mkDockWidget(m_main_window, &(*it)->widget(), (*it)->config().m_show, log_name);
		m_main_window->loadLayout(preset_name);
	}
	return it;
}


