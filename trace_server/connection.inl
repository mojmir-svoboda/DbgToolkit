#pragma once

inline void Connection::defaultConfigFor (logs::LogConfig & config)
{
	QString const & appname = getAppName();
	int const idx = findAppNameInMainWindow(appname);
	if (idx != e_InvalidItem)
	{
		convertBloodyBollockyBuggeryRegistry(config);
		return;
	}
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

		QString preset_name = m_main_window->matchClosestPresetName(getAppName());
		if (!preset_name.isEmpty())
		{
			QStringList const prs = preset_name.split("/");
			if (prs.size() == 2 && prs.at(0) == getAppName())
				preset_name = prs.at(1);
			else
				preset_name.clear();
		}
	
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
					QString const cfg_fname = getDataTagFileName(getConfig().m_appdir, getAppName(), s, preset_prefix, tag);
					if (existsFile(cfg_fname))
					{
						if (s == QString(g_defaultPresetName))
							default_present = true;
						candidates << s;
					}

					m_main_window->mentionInPresetHistory(test_preset_name);
				}

				if (default_present)
					preset_name = g_defaultPresetName;
				else
				{
					if (candidates.size())
						preset_name = candidates.at(0);
				}
			}
		}

		if (preset_name.isEmpty())
			preset_name = g_defaultPresetName; // fallback to default

		m_main_window->mentionInPresetHistory(preset_name);
		QString const fname = getDataTagFileName(getConfig().m_appdir, getAppName(), preset_name, preset_prefix, tag);

		
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
		dataptr_t const dd = new data_t(this, fname, path);
		it = m_data.get<TypeN>().insert(tag, dd);

		dd->m_config.m_tag = tag;
		if (!preset_name.isEmpty())
		{
			QString const cfg_path = getConfig().m_appdir + "/" + getAppName() + "/" + preset_name;
			dd->widget().loadConfig(cfg_path);
		}

		DockedWidgetBase & dwb = *dd;;

		dd->m_wd = m_main_window->m_dock_mgr.mkDockWidget(dwb, (*it)->config().m_show);
		dwb.m_wd->setWidget(&(*it)->widget());
		(*it)->widget().applyConfig();

		bool const visible = (*it)->config().m_show;
		//if (m_main_window->ganttState() == e_FtrEnabled && visible)
		dd->m_wd->setVisible(visible);
		dd->widget().setVisible(visible);
		QModelIndex const item_idx = m_main_window->m_dock_mgr.addDockedTreeItem(dwb, visible);

		if (/*m_main_window->ganttState() == e_FtrEnabled &&*/ visible)
		{
			dd->m_wd->setVisible(visible);
			dd->widget().setVisible(visible);

			bool const in_cw = (*it)->config().m_central_widget;
			if (in_cw)
				toCentralWidget(dd->m_wd, dd->m_widget, in_cw);
			else
				m_main_window->restoreDockWidget(dd->m_wd);
		}
		else
		{
			dd->m_wd->setVisible(false);
			dd->widget().setVisible(false);
			// @TODO: visible to data_tree?
			// @TODO: visible to widget conf? probably not..
		}
	}
	return it;
}


