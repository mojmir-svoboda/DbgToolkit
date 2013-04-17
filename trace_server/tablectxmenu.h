#pragma once
#include <QMenu>
#include <QWidget>
#include <QObject>
#include <QAction>
#include <QWidgetAction>
#include <QLineEdit>
#include <QLabel>
#include "config.h"
#include "ui_settingstable.h"
#include "qtsln/qtcolorpicker/qtcolorpicker.h"

namespace table {

	struct CtxTableConfig : QObject
	{
		TableConfig & m_pcfg;
		Ui::SettingsTable * m_ui;
		QDockWidget * m_widget;

		CtxTableConfig (TableConfig & cfg, QWidget * parent)
			: m_pcfg(cfg)
			, m_ui(new Ui::SettingsTable)
			, m_widget(new QDockWidget(parent))
		{
			qDebug("%s this=0x%08x", __FUNCTION__, this);
			m_widget->setVisible(false);
			m_ui->setupUi(m_widget);
		}

		~CtxTableConfig ()
		{
			qDebug("%s this=0x%08x", __FUNCTION__, this);

			m_widget->setVisible(false);
			delete m_ui;
			m_ui = 0;
			delete m_widget;
			m_widget = 0;
		}

		void onShowContextMenu (QPoint const & pos)
		{
			bool const visible = m_widget->isVisible();
			qDebug("%s visible=%u", __FUNCTION__, visible);
			m_widget->setVisible(!visible);

			if (m_widget->isVisible())
				m_widget->move(pos);
		}

		void onHideContextMenu ()
		{
			qDebug("%s", __FUNCTION__);
			m_widget->setVisible(false);
		}

		Ui::SettingsTable * ui () { return m_ui; }
	};

}

