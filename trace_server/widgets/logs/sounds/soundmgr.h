#pragma once
#include <widgets/logs/filters/filtermgr.h> // @TODO @FIXME
#include "sound_regex.h"

namespace logs {

struct SoundMgr : logs::FilterMgr
{
	SoundMgr (QWidget * parent = 0);
	virtual ~SoundMgr ();

	virtual E_FilterType type () const { return e_Sound_Mgr; }
	virtual void defaultConfig ();

	virtual void initUI ();
	virtual void doneUI ();
	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual bool accept (QModelIndex const & sourceIndex);
	virtual bool action (QModelIndex const & sourceIndex);
	virtual void clear ();
	virtual FilterBase * filterFactory (E_FilterType t, QWidget * parent);

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterMgr::serialize(ar, version);
	}

	//FilterXX *			getFilterXX () { return static_cast<FilterXX *>(m_cache[e_Filter_XX]); }
	//FilterXX const *		getFilterXX () const { return static_cast<FilterXX const *>(m_cache[e_Filter_XX]); }
	SoundRegex *			getSoundRegex () { return static_cast<SoundRegex *>(m_cache[e_Sound_Regex]); }
	SoundRegex const *		getSoundRegex () const { return static_cast<SoundRegex const *>(m_cache[e_Sound_Regex]); }

	//void clearUI ();
	//void setConfigToUI ();
	//void setUIToConfig ();

public slots:
signals:

public:
	Q_OBJECT
};

} // namespace logs