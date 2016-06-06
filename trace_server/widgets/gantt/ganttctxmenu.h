#pragma once
#include <QMenu>
#include <QWidget>
#include <QObject>
#include <QAction>
#include <QWidgetAction>
#include <QLineEdit>
#include <QLabel>
#include "ui_settingsgantt.h"
#include <3rd/qtsln/qtcolorpicker/qtcolorpicker.h>

namespace gantt {

	struct CtxGanttConfig : QObject
	{
		GanttConfig & m_pcfg;
		Ui::SettingsGantt * m_ui;
		QDockWidget * m_widget;

		CtxGanttConfig (GanttConfig & cfg, QWidget * parent)
			: m_pcfg(cfg)
			, m_ui(new Ui::SettingsGantt)
			, m_widget(new QDockWidget(parent))
		{
			m_widget->setVisible(false);
			m_ui->setupUi(m_widget);
		}

		~CtxGanttConfig ()
		{
			m_widget->setVisible(false);
			delete m_ui;
			m_ui = 0;
			delete m_widget;
			m_widget = 0;
		}

		void onShowContextMenu (QPoint const & pos)
		{
			bool const visible = m_widget->isVisible();
			m_widget->setVisible(!visible);

			if (m_widget->isVisible())
				m_widget->move(pos);
		}

		void onHideContextMenu ()
		{
			m_widget->setVisible(false);
		}

		Ui::SettingsGantt * ui () { return m_ui; }
	};

}

