#pragma once

struct FiltersCtxMenu : QObject {
		FiltersConfig & m_pcfg;
		Ui::SettingsFilters * ui_settingsfilters;
		QDockWidget * m_settingsfilters;

		FiltersCtxMenu (FiltersConfig & cfg, QWidget * parent)
			: m_pcfg(cfg)
			, ui_settingsfilters(new Ui::SettingsFilters)
			, m_settingsfilters(new QDockWidget(parent))
		{
			m_settingsfilters->setVisible(false);
			ui_settingsfilters->setupUi(m_settingsfilters);

			//ui_settingsfilters->color1Button->setStandardColors();
			//ui_settingsfilters->color2Button->setStandardColors();
			//ui_settingsfilters->color3Button->setStandardColors();
			//ui_settingsfilters->color4Button->setStandardColors();

			//setIntervalRange(ui_settingsfilters->beginSpinBox);
			//setIntervalRange(ui_settingsfilters->endSpinBox);
		}

		~FiltersCtxMenu ()
		{
			m_settingsfilters->setVisible(false);
			delete ui_settingsfilters;
			ui_settingsfilters = 0;
			delete m_settingsfilters;
			m_settingsfilters = 0;
		}

		void onShowContextMenu (QPoint const & pos)
		{
			bool const visible = m_settingsfilters->isVisible();
			m_settingsfilters->setVisible(!visible);

			if (m_settingsfilters->isVisible())
				m_settingsfilters->move(pos);
		}

		void onHideContextMenu ()
		{
			m_settingsfilters->setVisible(false);
		}

		Ui::SettingsFilters * ui () { return ui_settingsfilters; }

};
