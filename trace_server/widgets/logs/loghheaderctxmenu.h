#pragma once
#include <QWidget>

namespace logs {
	namespace Ui { class LogHHeaderCtxMenu; }
	class LogWidget;

	class LogHHeaderCtxMenu : public QWidget
	{
			Q_OBJECT

	public:
			explicit LogHHeaderCtxMenu (LogWidget & log_widget, QWidget *parent = 0);
			~LogHHeaderCtxMenu ();

			void init ();
			void setHeaderValuesToUI ();
			void onShowContextMenu (QPoint const & pos);
			void onHideContextMenu ()
			{
				setVisible(false);
			}

			Ui::LogHHeaderCtxMenu * ui () { return m_ui; }
			void clearUI ();

	public slots:
			void onClickedAtSettingColumnShow (QModelIndex const & idx);
			void onClickedAtColumn (QStandardItem *);
			void onAllButton ();
			void onNoneButton ();
			void onDefaultButton ();

	private:
			Ui::LogHHeaderCtxMenu * m_ui;
			LogWidget & m_log_widget;
	};
}
