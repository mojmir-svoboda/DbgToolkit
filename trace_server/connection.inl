#pragma once

template <int TypeN>
bool Connection::loadConfigFor (QString const & preset_name, typename SelectConfig<TypeN>::type & config, QString const & tag)
{
	char const * preset_prefix = g_fileTags[TypeN];
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, preset_prefix, tag);
	qDebug("loadCfgFor<>: load cfg file=%s", fname.toStdString().c_str());
	return loadConfig(config, fname);
}

inline void Connection::defaultConfigFor (logs::LogConfig & config)
{
	QString const & appname = getAppName();
	int const idx = findAppNameInMainWindow(appname);
	if (idx != e_InvalidItem)
	{
		convertBloodyBollockyBuggeryRegistry(config);
		return;
	}
	fillDefaultConfig(config);
}
inline void Connection::defaultConfigFor (plot::PlotConfig & config)
{
	fillDefaultConfig(config);
}
inline void Connection::defaultConfigFor (table::TableConfig & config)
{
	fillDefaultConfig(config);
}
inline void Connection::defaultConfigFor (gantt::GanttConfig & config)
{
	fillDefaultConfig(config);
}
inline void Connection::defaultConfigFor (FrameViewConfig & config)
{
	fillDefaultConfig(config);
}

template <int TypeN>
void Connection::defaultConfigFor (typename SelectConfig<TypeN>::type & config, QString const & tag)
{
	defaultConfigFor(config);
	config.m_tag = tag;
}

template <int TypeN>
typename SelectIterator<TypeN>::type  Connection::dataWidgetFactory (QString const tag)
{
	char const * preset_prefix = g_fileTags[TypeN];
	QString const log_name = getAppName() + "/" + preset_prefix + "/" + tag;

	typedef typename SelectIterator<TypeN>::type iterator;
	iterator it = m_data.get<TypeN>().find(tag);
	if (it == m_data.get<TypeN>().end())
	{
		qDebug("datawidget factory: creating type=%i with %s", TypeN, tag.toStdString().c_str());

		typedef typename SelectWidget<TypeN>::type widget_t;
		typedef typename SelectConfig<TypeN>::type config_t;

		config_t template_config;
		template_config.m_tag = tag;

		QString preset_name = m_main_window->matchClosestPresetName(getAppName());
		if (preset_name.isEmpty())
		{
			QStringList subdirs;
			if (int const n = findPresetsForApp(getConfig().m_appdir, getAppName(), subdirs))
			{
				bool default_present = false;
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

					m_main_window->mentionInPresetHistory(test_preset_name);
				}

				if (default_present)
					preset_name = getAppName() + "/" + g_defaultPresetName;
				else
				{
					if (candidates.size())
						preset_name = candidates.at(0);
				}
			}
		}

		if (preset_name.isEmpty())
		{
			preset_name = getPresetPath(getAppName(), g_defaultPresetName); // fallback to default
		}

		m_main_window->mentionInPresetHistory(preset_name);
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, preset_prefix, tag);
		if (!preset_name.isEmpty())
		{
			bool const loaded = loadConfigFor<TypeN>(preset_name, template_config, tag);
			if (!loaded)
			{
				defaultConfigFor<TypeN>(template_config, tag);
			}
		}
		
		typedef typename SelectDockedData<TypeN, dockeddata_t>::type data_t;
		typedef typename SelectDockedData<TypeN, dockeddataptr_t>::type dataptr_t;
		QStringList path;
		QString const & name0 = m_main_window->dockedName();
		QString const & name1 = getAppName();
		QString const & name2 = preset_prefix;
		QString const & name3 = tag;
		path.append(name0);
		path.append(name1);
		path.append(name2);
		path.append(name3);
		dataptr_t const dd = new data_t(this, template_config, fname, path);
		it = m_data.get<TypeN>().insert(tag, dd);

		DockedWidgetBase & dwb = *dd;;

		dd->m_wd = m_main_window->m_dock_mgr.mkDockWidget(dwb, (*it)->config().m_show);
		dwb.m_wd->setWidget(&(*it)->widget());
		m_main_window->loadLayout(preset_name);

		bool const visible = (*it)->config().m_show;

		//if (m_main_window->ganttState() == e_FtrEnabled && visible)
		dd->m_wd->setVisible(visible);
		dd->widget().setVisible(visible);
		QModelIndex const item_idx = m_main_window->m_dock_mgr.addDockedTreeItem(dwb, visible);
	}
	return it;
}


