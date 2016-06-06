#include "soundmgr.h"
#include <algorithm>
#include <QGridLayout>
#include <QComboBox>
#include <QModelIndexList>
#include "ui_combolist.h"
#include <serialize/serialize.h>

namespace logs {

bool SoundMgr::accept (QModelIndex const & sourceIndex)
{
	return true;
}

bool SoundMgr::action (QModelIndex const & idx)
{
	bool accepted = true;
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
	{
		FilterBase * b = m_filters[i];
		if (b->enabled())
		{
			bool const filter_accepted = b->action(idx); // @TODO: short circuit?
			accepted |= filter_accepted;
		}
	}
	return accepted;
}

// @TODO: to base l8r (tag to virtual fn)
void SoundMgr::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_soundTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();

	recreateFilters();

	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->loadConfig(path);
}

void SoundMgr::saveConfig (QString const & path)
{
	m_currTab = m_tabFilters->currentIndex();
	QString const fname = path + "/" + g_soundTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);

	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->saveConfig(path);
}

FilterBase * SoundMgr::filterFactory (E_FilterType t, QWidget * parent)
{
	switch (t)
	{
		case e_Sound_Mgr: return new SoundMgr (parent);
		case e_Sound_Regex: return new SoundRegex (parent);
		default: return 0;
	}
}

/////////////////// FILTER MGR ///////////////////////////////

SoundMgr::SoundMgr (QWidget * parent)
	: FilterMgr(parent)
{
	m_cache.resize(e_filtertype_max_value);
}

SoundMgr::~SoundMgr ()
{
	qDebug("%s", __FUNCTION__);
}

void SoundMgr::defaultConfig ()
{
	m_filter_order.clear();
	m_filter_order.push_back(g_filterNames[e_Sound_Regex]);
}

/////////////////////////////////////////////////////////////////////

void SoundMgr::initUI ()
{
	FilterMgr::initUI();
	setObjectName(QStringLiteral("SoundWidget"));
	//m_widget->setObjectName(QStringLiteral("FilterWidget"));
}

void SoundMgr::doneUI ()
{
	FilterMgr::doneUI();
}

void SoundMgr::clear ()
{
}

}