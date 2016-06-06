#include "filter_script.h"
#include "constants.h"
#include <serialize/serialize.h>
#include <utils/utils_qstandarditem.h>
#include <boost/function.hpp>
#include <QPainter>
//#include <QScriptEngine>

FilterScript::FilterScript (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterScript)
	, m_data()
	, m_se(0)
	, m_model(0)
	, m_delegate(0)
{
	initUI();
	setupModel();
	//m_se = new QScriptEngine();
}

FilterScript::~FilterScript ()
{
	destroyModel();
	doneUI();
}

void FilterScript::initUI ()
{
	m_ui->setupUi(this);
}

void FilterScript::doneUI ()
{
}

bool FilterScript::accept (QModelIndex const & sourceIndex)
{
	if (m_data.size() > 0)
	{
		/*QString msg;
		if (!cmd.getString(proto::tag_msg, msg))
			return true;*/

		for (int i = 0, ie = m_data.size(); i < ie; ++i)
		{
			FilteredScript const & fr = m_data.at(i);
			/*if (fr.match(msg))
			{
				if (!fr.m_is_enabled)
					continue;
				else
				{
					if (fr.m_state)
						return true;
					else
						return false;
				}
			}*/
		}
		return false;
	}
	return true; // no strings at all
}

void FilterScript::defaultConfig ()
{
	m_data.clear();
}

void FilterScript::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterScript::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterScript::applyConfig ()
{
	FilterBase::applyConfig();
}

void FilterScript::clear ()
{
}


///////////////////
void FilterScript::setupModel ()
{
}

void FilterScript::destroyModel ()
{
}

void FilterScript::setScriptState (QString const & s, int state)
{
}
void FilterScript::setScriptChecked (QString const & s, bool checked)
{
}
void FilterScript::removeFromScriptFilters (QString const & s)
{
}
void FilterScript::appendToScriptFilters (QString const & s, bool enabled, int state)
{
}

void FilterScript::appendToScriptWidgets (FilteredScript const & flt)
{
}

void FilterScript::onClickedAtScriptList (QModelIndex idx)
{
}

void FilterScript::recompile ()
{ }

void FilterScript::onScriptRm ()
{
}

void FilterScript::onScriptAdd ()
{
}

void FilterScript::locateItem (QString const & item, bool scrollto, bool expand)
{
}

//////// delegate
void ScriptDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
}





