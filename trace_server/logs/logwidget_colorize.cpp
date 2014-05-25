#include "logwidget.h"
#include <QStatusBar>
#include "logs/logtablemodel.h"
#include <logs/filterproxymodel.h>
#include <logs/findproxymodel.h>
#include "utils.h"
#include <serialize.h>
#include "connection.h"
#include "mainwindow.h"
#include "warnimage.h"
#include "colorizewidget.h"

namespace logs {

void LogWidget::onColorize ()
{
	ColorizeConfig & cfg = m_config.m_colorize_config;
	m_colorize_widget->applyConfig(cfg);

	m_colorize_widget->onActivate();
	m_colorize_widget->setFocusProxy(this); // dunno what the proxies are for
}

void LogWidget::onColorizeNext ()
{
	m_colorize_widget->onColorizeNext();
}

void LogWidget::onColorizePrev ()
{
	m_colorize_widget->onColorizePrev();
}

/*void LogWidget::onColorizeAllRefs ()
{
	//m_colorize_widget->onColorizeAllRefs();
}
*/

void LogWidget::handleColorizeAction (ColorizeConfig const & fc)
{
	bool const select_only = !fc.m_refs && !fc.m_clone;

	if (fc.m_regexp)
	{
		if (fc.m_regexp_val.isEmpty())
			return;
		if (!fc.m_regexp_val.isValid())
			return;
	}

	saveColorizeConfig();

	if (select_only)
	{
/*		if (fc.m_next)
			findAndSelectNext(fc);
		else if (fc.m_prev)
			findAndSelectPrev(fc);
		else
			findAndSelect(fc);*/
	}
	else
	{
	}
}


}

