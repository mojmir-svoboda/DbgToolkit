#pragma once
#include <serialize.h>


inline QString Connection::getClosestPresetName (E_DataWidgetType type, QString const & tag)
{
	return QString("default");
}

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
		m_curr_preset = getAppName() + "/" + preset_name;
		
		typedef typename SelectDockedData<TypeN, dockeddata_t>::type data_t;
		typedef typename SelectDockedData<TypeN, dockeddataptr_t>::type dataptr_t;
		QStringList path;
		mkWidgetPath<TypeN>(tag, path);
		widget_t * const dd = new widget_t(this, fname, path);
		it = m_data.get<TypeN>().insert(tag, dd);

		dd->m_config.m_tag = tag;
		if (!preset_name.isEmpty())
		{
			QString const cfg_path = getGlobalConfig().m_appdir + "/" + m_curr_preset;
			dd->widget().loadConfig(cfg_path);
		}

		m_main_window->mentionInPresetHistory(m_curr_preset);
		m_main_window->setPresetAsCurrent(m_curr_preset);

		DockedWidgetBase & dwb = *dd;

		dd->m_wd = m_main_window->m_dock_mgr.mkDockWidget(dwb, (*it)->config().m_show);
		dwb.m_wd->setWidget(&(*it)->widget());
        dd->widget().setDockedWidget(dd);
		//(*it)->widget().applyConfig(); handled by caller

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
				getMainWindow()->onDockRestoreButton();
				//m_main_window->restoreDockWidget(dd->m_wd);
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

template <class ConfigT>
bool Connection::dataWidgetConfigPreload (QString const tag, ConfigT & config)
{
	QString const preset_name = getClosestPresetName<TypeN>(tag);
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

		QString const preset_name = getClosestPresetName<TypeN>(tag);
		char const * widget_prefix = g_fileTags[TypeN];
		char const * widget_fname = g_fileNames[TypeN];
		QString const fname = mkWidgetFileName(getGlobalConfig().m_appdir, getAppName(), preset_name, widget_prefix, tag, widget_fname);
		
		typedef typename SelectDockedData<TypeN, dockeddata_t>::type data_t;
		typedef typename SelectDockedData<TypeN, dockeddataptr_t>::type dataptr_t;
		QStringList path;
		mkWidgetPath<TypeN>(tag, path);
		dataptr_t const dd = new data_t(this, fname, path, config);
		it = m_data.get<TypeN>().insert(tag, dd);
		dd->m_config.m_tag = tag;
		DockedWidgetBase & dwb = *dd;;
		dd->m_wd = m_main_window->m_dock_mgr.mkDockWidget(dwb, (*it)->config().m_show);
		dwb.m_wd->setWidget(&(*it)->widget());
        dd->widget().setDockedWidget(dd);
		// no applyConfig here!

		bool const visible = (*it)->config().m_show;
		dd->m_wd->setVisible(visible);
		dd->widget().setVisible(visible);
		QModelIndex const item_idx = m_main_window->m_dock_mgr.addDockedTreeItem(dwb, visible);

		if (visible)
		{
			dd->m_wd->setVisible(visible);
			dd->widget().setVisible(visible);

			bool const in_cw = (*it)->config().m_central_widget;
			if (in_cw)
				toCentralWidget(dd->m_wd, dd->m_widget, in_cw);
			else
			{
				getMainWindow()->onDockRestoreButton();
				//m_main_window->restoreDockWidget(dd->m_wd);
			}
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



