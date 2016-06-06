#pragma once
#include <QMenu>
#include <QWidget>
#include <QObject>
#include <QAction>
#include <QWidgetAction>
#include <QLineEdit>
#include <QLabel>
#include "ui_settingsframeview.h"
#include <3rd/qtsln/qtcolorpicker/qtcolorpicker.h>

namespace frameview {

	struct CtxFrameViewConfig : QObject
	{
		FrameViewConfig & m_pcfg;
		Ui::SettingsFrameView * ui_settingsframeview;
		QDockWidget * m_settingsframeview;

		CtxFrameViewConfig (FrameViewConfig & cfg, QWidget * parent)
			: m_pcfg(cfg)
			, ui_settingsframeview(new Ui::SettingsFrameView)
			, m_settingsframeview(new QDockWidget(parent))
		{
			m_settingsframeview->setVisible(false);
			ui_settingsframeview->setupUi(m_settingsframeview);

			ui_settingsframeview->color1Button->setStandardColors();
			ui_settingsframeview->color2Button->setStandardColors();
			ui_settingsframeview->color3Button->setStandardColors();
			ui_settingsframeview->color4Button->setStandardColors();

			setIntervalRange(ui_settingsframeview->beginSpinBox);
			setIntervalRange(ui_settingsframeview->endSpinBox);
		}

		void setIntervalRange (QDoubleSpinBox * w)
		{
			w->setMinimum(-1e6);
			w->setMaximum(1e6);
		}

		~CtxFrameViewConfig ()
		{
			m_settingsframeview->setVisible(false);
			delete ui_settingsframeview;
			ui_settingsframeview = 0;
			delete m_settingsframeview;
			m_settingsframeview = 0;
		}

		void onShowContextMenu (QPoint const & pos)
		{
			bool const visible = m_settingsframeview->isVisible();
			m_settingsframeview->setVisible(!visible);

			if (m_settingsframeview->isVisible())
				m_settingsframeview->move(pos);
		}

		void onHideContextMenu ()
		{
			m_settingsframeview->setVisible(false);
		}

		Ui::SettingsFrameView * ui () { return ui_settingsframeview; }
	};

}

