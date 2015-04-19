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
#include "find_utils_table.h"

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

void LogWidget::handleColorizeAction (ColorizeConfig const & cc)
{
	bool const select_only = !cc.m_refs && !cc.m_clone;

	if (cc.m_regexp)
	{
		if (cc.m_regexp_val.pattern().isEmpty())
			return;
		if (!cc.m_regexp_val.isValid())
			return;
	}

	saveColorizeConfig();

	if (select_only)
	{
		if (cc.m_regexp)
		{
			colorizerMgr()->mkFilter(e_Colorizer_Regex);
			colorizerMgr()->getColorizerRegex()->add(cc.m_str, cc.m_fgcolor, cc.m_bgcolor);
			//onInvalidateFilter(); //@TODO: should be done by filter?
		}
		else
		{
			colorizerMgr()->mkFilter(e_Colorizer_String);
			colorizerMgr()->getColorizerString()->add(cc.m_str, cc.m_fgcolor, cc.m_bgcolor);
			//onInvalidateFilter(); //@TODO: should be done by filter?
		}

		if (cc.m_next)
			findAndSelectNext(m_tableview, cc);
		else if (cc.m_prev)
			findAndSelectPrev(m_tableview, cc);
		//else
			//findAndSelect(cc)
	}
	else
	{
	}
}


}

