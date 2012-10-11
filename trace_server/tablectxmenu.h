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
		Ui::SettingsTable * ui_settingstable;
		QDockWidget * m_settingstable;

		CtxTableConfig (TableConfig & cfg, QWidget * parent)
			: m_pcfg(cfg)
			, ui_settingstable(new Ui::SettingsTable)
			, m_settingstable(new QDockWidget(parent))
		{
			qDebug("%s this=0x%08x", __FUNCTION__, this);
			m_settingstable->setVisible(false);
			ui_settingstable->setupUi(m_settingstable);
		}

		~CtxTableConfig ()
		{
			qDebug("%s this=0x%08x", __FUNCTION__, this);

			m_settingstable->setVisible(false);
			delete ui_settingstable;
			ui_settingstable = 0;
			delete m_settingstable;
			m_settingstable = 0;
		}

		void onShowContextMenu (QPoint const & pos)
		{
			qDebug("%s", __FUNCTION__);
			bool const visible = m_settingstable->isVisible();
			m_settingstable->setVisible(!visible);

			if (m_settingstable->isVisible())
				m_settingstable->move(pos);
		}

		void onHideContextMenu ()
		{
			qDebug("%s", __FUNCTION__);
			m_settingstable->setVisible(false);
		}

		Ui::SettingsTable * ui () { return ui_settingstable; }
	};

}

