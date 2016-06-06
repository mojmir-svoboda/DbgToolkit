#pragma once
#include <QMenu>
#include <QWidget>
#include <QObject>
#include <QAction>
#include <QWidgetAction>
#include <QLineEdit>
#include <QLabel>
#include "ui_settingslog.h"
#include <3rd/qtsln/qtcolorpicker/qtcolorpicker.h>

namespace logs {

	class LogTableView;
	struct LogConfig;

	struct LogCtxMenu : QObject
	{
		LogWidget & m_log_widget;
		::Ui::SettingsLog * m_ui;
		QDockWidget * m_widget;
		LogCtxMenu (LogWidget & cfg, QWidget * parent);

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
			//if (state == Qt::Checked)
			//	m_ui->inViewCheckBox->setCheckState(Qt::Unchecked);
		}

		~LogCtxMenu ()
		{
			m_widget->setVisible(false);
			delete m_ui;
			m_ui = 0;
			delete m_widget;
			m_widget = 0;
		}

		void onShowContextMenu (QPoint const & pos);

		void setConfigValuesToUI (LogConfig const & cfg);
		void refreshUI ();

		void onHideContextMenu ()
		{
			m_widget->setVisible(false);
		}

		::Ui::SettingsLog * ui () { return m_ui; }
		void clearUI ();

	public slots:
		//void setupSeparatorChar (QString const & c);
		//QString separatorChar () const;
		void syncSettingsViews (QListView const * const invoker, QModelIndex const idx);
		void onClickedAtSettingColumnSetup (QModelIndex const idx);
		void onClickedAtSettingColumnSizes (QModelIndex const idx);
		void onClickedAtSettingColumnAlign (QModelIndex const idx);
		void onClickedAtSettingColumnElide (QModelIndex const idx);
		//void onSettingsAppSelectedTLV (bool const first_time);
		void onSettingsAppSelectedCSV (int const columns, bool const first_time);
		void onClickedAtAutoSetupButton ();
		void onClickedAtAutoSetupButton_noreorder ();
		void onClickedAtApplyButton ();
		void onClickedAtSaveButton ();
		void onClickedAtCancelButton ();
		void onCommitTagData (QString const &);

	protected:
		void prepareSettingsWidgets ();

	Q_OBJECT
	};

}

