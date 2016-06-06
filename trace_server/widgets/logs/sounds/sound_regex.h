#pragma once
#include <filters/filterbase.h>
#include "ui_sound_regex.h"
#include <boost/serialization/nvp.hpp>
#include "config.h"
#include "wavetableconfig.h"
#include <vector>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <widgets/logs/logtablemodel.h>
struct WaveTable;

struct SoundNotif
{
	bool	m_is_enabled;
	bool m_is_regex;
	bool	m_case_sensitive;
	bool	m_whole_word;
	QString m_regex_str;
	QRegularExpression m_regex;
	CheckedComboBoxConfig m_where;
  WaveConfig m_waveconfig;

	bool isValid () const { return m_is_regex ? m_regex.isValid() : true; }

	bool accept (QString const & str) const
	{
		if (m_is_enabled)
		{
			if (m_is_regex)
			{
				if (m_regex.isValid())
				{
					QRegularExpressionMatch m = m_regex.match(str);
					return m.hasMatch();
				}
				else
					return false;
			}
			else
			{
				bool const res = str.contains(m_regex_str, m_case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
				return res;
			}
		}
		return false;
	}

	SoundNotif () { }
	SoundNotif (bool is_regex, bool cs, bool ww, QString const & rs, CheckedComboBoxConfig & cccfg, WaveConfig & wc)
        : m_is_enabled(0), m_is_regex(is_regex), m_case_sensitive(cs), m_whole_word(ww), m_regex_str(rs), m_regex(rs), m_where(cccfg), m_waveconfig(wc)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("is_regex", m_is_regex);
		ar & boost::serialization::make_nvp("case_sensitive", m_case_sensitive);
		ar & boost::serialization::make_nvp("whole_word", m_whole_word);
		ar & boost::serialization::make_nvp("regex_str", m_regex_str);
		ar & boost::serialization::make_nvp("regex", m_regex);
		ar & boost::serialization::make_nvp("waveconfig", m_waveconfig);
		ar & boost::serialization::make_nvp("where", m_where);
	}
};


struct SoundRegex : FilterBase
{
	Ui_SoundRegex * m_ui;

	SoundRegex (QWidget * parent = 0);
	virtual ~SoundRegex ();

	virtual void initUI () override;
	virtual void doneUI () override;

	virtual E_FilterType type () const override { return e_Sound_Regex; }

/*	virtual bool accept (QModelIndex const & sourceIndex) const override;*/
	virtual bool action (QModelIndex const & sourceIndex) override;

	virtual void defaultConfig () override;
	virtual void loadConfig (QString const & path) override;
	virtual void saveConfig (QString const & path) override;
	virtual void applyConfig () override;
	virtual void clear () override;

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterBase::serialize(ar, version);
		ar & boost::serialization::make_nvp("colorizer_regex", m_data);
	}

	// color_regex specific
	SoundNotif & findOrCreateSoundNotif (QString const & str);
	void setupModel ();
	void setupModelHeader ();
	void destroyModel ();
	SoundNotif const * findMatch (QString const & item) const;
	//void append (QString const & item);
	void remove (QString const & item);
	void recompile ();
	void setConfigToUI ();
	//void setConfigToUI (SoundNotif const & cfg);
	void setDefaultSearchConfig (CheckedComboBoxConfig const & cccfg);
	void setWaveTable (WaveTable * wavetable) { m_wavetable = wavetable; }

	void addRowToUI (SoundNotif const & cfg);
	SoundNotif & add (QString const & regex);
	//void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }
	void onNotifButtonChanged (int role);

	void actionNotify (QModelIndex const & sourceIndex, SoundNotif const & ct) const;
	void recompileNotifRegexps ();
	void recompileNotifRegex (SoundNotif & ct);

	typedef std::vector<SoundNotif> filters_t;
	filters_t				m_data;
	QStandardItemModel *	m_model;
	CheckedComboBoxConfig m_cccfg;
	WaveTable * m_wavetable;

	Q_OBJECT
public slots:
	void onClickedAt (QModelIndex idx);
	void onSelectAll ();
	void onSelectNone ();
	void onAdd ();
	void onRm ();
	void onActivate (int);
	void onDataChanged (QModelIndex const &, QModelIndex const &);

signals:
};
