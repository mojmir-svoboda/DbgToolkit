#pragma once
#include <QMenu>
#include <QWidget>
#include <QObject>
#include <QAction>
#include <QWidgetAction>
#include <QLineEdit>
#include <QLabel>
#include "ui_settingslog.h"
#include "../qtsln/qtcolorpicker/qtcolorpicker.h"

namespace logs {

	struct CtxLogConfig : QObject
	{
		LogConfig & m_pcfg;
		Ui::SettingsLog * m_ui;
		QDockWidget * m_widget;

		CtxLogConfig (LogConfig & cfg, QWidget * parent)
			: m_pcfg(cfg)
			, m_ui(new Ui::SettingsLog)
			, m_widget(new QDockWidget(parent))
		{
			m_widget->setVisible(false);
			m_ui->setupUi(m_widget);

		}

		void onInViewStateChanged (int state)
		{
			if (state == Qt::Checked)
			{
				m_ui->autoScrollCheckBox->setCheckState(Qt::Unchecked);
				//onNextToView();
			}
		}

		void onAutoScrollStateChanged (int state)
		{
			if (state == Qt::Checked)
				m_ui->inViewCheckBox->setCheckState(Qt::Unchecked);
		}

		~CtxLogConfig ()
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

		Ui::SettingsLog * ui () { return m_ui; }
	};
}

