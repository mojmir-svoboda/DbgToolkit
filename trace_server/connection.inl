#pragma once

template <int TypeN>
typename SelectIterator<TypeN>::type Connection::dataWidgetFactory (QString const tag)
{
	typedef typename SelectIterator<TypeN>::type iterator;
	iterator it = m_data.get<TypeN>().find(tag);
	if (it == m_data.get<TypeN>().end())
	{
		qDebug("datawidget factory: creating type=%i with %s", TypeN, tag.toStdString().c_str());

		typedef typename SelectWidget<TypeN>::type widget_t;
		typedef typename SelectConfig<TypeN>::type config_t;

		QString const preset_name = getClosestPresetName(tag);
		char const * widget_prefix = g_fileTags[TypeN];
		char const * widget_fname = g_fileNames[TypeN];
		QString const fname = mkWidgetFileName(getGlobalConfig().m_appdir, getAppName(), preset_name, widget_prefix, tag, widget_fname);
		m_curr_preset = preset_name;
		
		QStringList path;
		mkWidgetPath(static_cast<E_DataWidgetType>(TypeN), tag, path);
		widget_t * const dd = new widget_t(this, fname, path);
		it = m_data.get<TypeN>().insert(tag, dd);

		dd->config().m_tag = tag;
		if (!preset_name.isEmpty())
		{
			QString const cfg_path = mkAppPresetPath(getGlobalConfig().m_appdir, getAppName(), m_curr_preset);
			dd->loadConfig(cfg_path);
		}

		//@TODO: pozdeji
		//m_main_window->mentionInPresetHistory(m_curr_preset);
		//m_main_window->setPresetAsCurrent(m_curr_preset);

		DockedWidgetBase & dwb = *dd;

		DockWidget * dockwidget = m_main_window->m_dock_mgr.mkDockWidget(dwb, (*it)->config().m_show);
		//dwb.m_wd->setWidget(&(*it)->widget());
        //dd->widget().setDockedWidget(dd);
		//(*it)->widget().applyConfig(); handled by caller

		bool const visible = (*it)->config().m_show;
		//if (m_main_window->ganttState() == e_FtrEnabled && visible)
		//dd->m_wd->setVisible(visible);
		dd->setVisible(visible);
		QModelIndex const item_idx = m_main_window->m_dock_mgr.addDockedTreeItem(dwb, visible);

		if (/*m_main_window->ganttState() == e_FtrEnabled &&*/ visible)
		{
			//dd->m_wd->setVisible(visible);
			dd->setVisible(visible);

			getMainWindow()->onDockRestoreButton();
			//m_main_window->restoreDockWidget(dd->m_wd);
		}
		else
		{
			//dd->m_wd->setVisible(false);
			dd->setVisible(false);
			// @TODO: visible to data_tree?
			// @TODO: visible to widget conf? probably not..
		}
	}
	return it;
}

/*template <int TypeN>
void Connection::removeDockWidget (std::tuple_element<TypeN, data_widgets_t>::iterator ptr)
{
    foreach (QString key, m_data.get<TypeN>().keys() )
    {
        if (m_data.get<TypeN>().value(key) == ptr)
        {
            m_data.get<TypeN>().remove(key);
            return;
        }
    }
}*/

/*typename std::tuple_element<TypeN, data_widgets_t>::iterator Connection::findDockedWidget (E_DataWidgetType type, QString const & tag)
#include <boost/tuple/tuple.hpp>
{
	typedef typename SelectIterator<TypeN>::type iterator;
	iterator it = m_data.get<TypeN>().find(tag);
    return it;
}*/

template <int TypeN>
bool Connection::dataWidgetConfigPreload (QString const tag, typename SelectConfig<TypeN>::type & config)
{
	QString const preset_name = getClosestPresetName(tag);
	char const * widget_prefix = g_fileTags[TypeN];
	char const * widget_fname = g_fileNames[TypeN];
	QString const fname = mkWidgetFileName(getGlobalConfig().m_appdir, getAppName(), preset_name, widget_prefix, tag, widget_fname);
	
	if (!preset_name.isEmpty())
	{
		return ::loadConfigTemplate(config, fname);
	}
	return false;
}

template <int TypeN>
typename SelectIterator<TypeN>::type Connection::dataWidgetFactoryFrom (QString const tag, typename SelectConfig<TypeN>::type const & config)
{
	typedef typename SelectIterator<TypeN>::type iterator;
	iterator it = m_data.get<TypeN>().find(tag);
	if (it == m_data.get<TypeN>().end())
	{
		qDebug("datawidget factory: creating type=%i with %s", TypeN, tag.toStdString().c_str());

		typedef typename SelectWidget<TypeN>::type widget_t;
		typedef typename SelectConfig<TypeN>::type config_t;

		QString const preset_name = getClosestPresetName(tag);
		char const * widget_prefix = g_fileTags[TypeN];
		char const * widget_fname = g_fileNames[TypeN];
		QString const fname = mkWidgetFileName(getGlobalConfig().m_appdir, getAppName(), preset_name, widget_prefix, tag, widget_fname);
		
		QStringList path;
		mkWidgetPath(static_cast<E_DataWidgetType>(TypeN), tag, path);
		widget_t * const dd = new widget_t(this, fname, path);
		it = m_data.get<TypeN>().insert(tag, dd);
		dd->config().m_tag = tag;
		DockedWidgetBase & dwb = *dd;
		DockWidget * dw = m_main_window->m_dock_mgr.mkDockWidget(dwb, (*it)->config().m_show);
		// no applyConfig here!

		bool const visible = (*it)->config().m_show;
		//dd->m_wd->setVisible(visible);
		dd->setVisible(visible);
		QModelIndex const item_idx = m_main_window->m_dock_mgr.addDockedTreeItem(dwb, visible);

		if (visible)
		{
			//dd->m_wd->setVisible(visible);
			dd->setVisible(visible);

			getMainWindow()->onDockRestoreButton();
			//m_main_window->restoreDockWidget(dd->m_wd);
		}
		else
		{
			//dd->m_wd->setVisible(false);
			dd->setVisible(false);
			// @TODO: visible to data_tree?
			// @TODO: visible to widget conf? probably not..
		}
	}
	return it;
}



