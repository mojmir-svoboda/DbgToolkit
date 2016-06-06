#pragma once
#include <QtMultimedia/QSoundEffect>
#include "wavetableconfig.h"

struct Wave
{
	WaveConfig m_config;
	std::unique_ptr<QSoundEffect> m_snd;

	Wave (WaveConfig & cfg)
		: m_config(cfg)
		, m_snd(new QSoundEffect)
	{
		apply(cfg);
	}

	Wave ()
		: m_config()
		, m_snd(new QSoundEffect)
	{ }

	~Wave () { }

	void play () { m_snd->play(); }

	void apply (WaveConfig & cfg)
	{
		m_config = cfg;
		if (existsFile(cfg.m_fpath))
		{
			m_snd->setSource(QUrl::fromLocalFile(m_config.m_fpath));
		}
	}
};

struct WaveTable
{
  WaveTableConfig m_config;
  std::vector<std::unique_ptr<Wave>> m_wavetable;
	using iterator = std::vector<std::unique_ptr<Wave>>::iterator;

	void apply (WaveTableConfig & cfg)
	{
		m_config.clear();

		m_config = cfg;
		for (WaveConfig & wc : cfg.m_waves)
		{
			std::unique_ptr<Wave> w(new Wave(wc));
			m_wavetable.push_back(std::move(w));
		}
	}
	
	bool play (QString const & snd, float vol, int loop)
	{
		iterator it = find(snd);
		if (it != m_wavetable.end())
		{
			std::unique_ptr<Wave> & w = *it;
			if (!w->m_snd->isPlaying())
			{
				qDebug("snd: will play %s file=%s with vol=%f", w->m_config.m_name.toStdString().c_str(), w->m_config.m_fpath.toStdString().c_str(), vol);
				w->m_snd->play();
			}
			else
				qWarning("snd: already playing %s file=%s with vol=%f", w->m_config.m_name.toStdString().c_str(), w->m_config.m_fpath.toStdString().c_str(), vol);
			return true;
		}
		return false;
	}

	iterator find (QString const & name)
	{
		for (auto it = m_wavetable.begin(), ite = m_wavetable.end(); it != ite; ++it)
		{
			QString const & wavpath = (*it)->m_config.m_fpath;
			QString const & wavname = (*it)->m_config.m_name;
			if (wavpath.contains(name) || wavname.contains(name))
				return it;
		}
		return m_wavetable.end();
	}
};

inline void setValuesToUI (QComboBox * cb, WaveTableConfig const & cfg, WaveConfig const & wc)
{
	if (cb)
	{
		cb->clear();

		for (size_t i = 0, ie = cfg.m_waves.size(); i < ie; ++i)
		{
			QStandardItem * const item = new QStandardItem(cfg.m_waves[i].m_name);
			item->setToolTip(cfg.m_waves[i].m_fpath);
			//item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			//bool const on = cfg.m_states[i];
			//item->setData(on ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
			QStandardItemModel * m = qobject_cast<QStandardItemModel*>(cb->model());
			m->insertRow(cb->count(), item);
		}

		if (wc.m_name.isEmpty())
			cb->setCurrentIndex(0);
		else
			cb->setCurrentText(wc.m_name);
	}
}

inline void setUIValuesToConfig (QComboBox const * cb, WaveTableConfig const & cfg, WaveConfig & wc)
{
	int const cb_idx = cb->currentIndex();
	if (cb_idx >= 0)
	{
		size_t const idx = static_cast<size_t>(cb_idx);
		if (idx < cfg.m_waves.size())
		{
			WaveConfig const & wc_src = cfg.m_waves[idx];
			// @TODO check if cb->currentText == wc.m_name
			wc = wc_src;
		}
	}
}

