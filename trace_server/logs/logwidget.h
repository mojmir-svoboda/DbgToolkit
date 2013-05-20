#pragma once
#include <QWidget>
#include "../tableview.h"
#include "logconfig.h"
#include "logctxmenu.h"

class Connection;

namespace logs {

	class LogWidget : public TableView
	{
		Q_OBJECT
	public:
		LogWidget (Connection * oparent, QWidget * wparent, LogConfig & cfg, QString const & fname);

		void applyConfig (LogConfig & pcfg);
		virtual ~LogWidget ();

		LogConfig & getConfig () { return m_config; }
		LogConfig const & getConfig () const { return m_config; }

	protected:

		bool filterEnabled () const { return m_main_window->filterEnabled(); }

	void findTextInAllColumns (QString const & text, int from_row, int to_row, bool only_first)
	bool matchTextInCell (QString const & text, int row, int col)
	void endOfSearch ()
	void findTextInColumn (QString const & text, int col, int from_row, int to_row)
	void findTextInColumnRev (QString const & text, int col, int from_row, int to_row)
	void selectionFromTo (int & from, int & to) const
	void findAllTexts (QString const & text)
	void findText (QString const & text, tlv::tag_t tag)
	void findText (QString const & text)
	void findNext (QString const & text)
	void findPrev (QString const & text)
	QString findString4Tag (tlv::tag_t tag, QModelIndex const & row_index) const
	QVariant findVariant4Tag (tlv::tag_t tag, QModelIndex const & row_index) const
	void scrollToCurrentTag ()
	void scrollToCurrentSelection ()
	void scrollToCurrentTagOrSelection ()
	void nextToView ()
	void onFindFileLine (QModelIndex const &)


	public slots:

		void onShow ();
		void onHide ();
		void onHideContextMenu ();
		void onShowContextMenu (QPoint const & pos);
		void setConfigValuesToUI (LogConfig const & cfg);
		void setUIValuesToConfig (LogConfig & cfg);
		void onSaveButton ();
		void onApplyButton ();
		void onResetButton ();
		void onDefaultButton ();
		//void scrollTo (QModelIndex const & index, ScrollHint hint);
		
		//void performTimeSynchronization (int sync_group, unsigned long long time, void * source);
		//void performFrameSynchronization (int sync_group, unsigned long long frame, void * source);

		void onClearAllDataButton ();
		void onSectionResized (int logicalIndex, int oldSize, int newSize);

	signals:
		//void requestTimeSynchronization (int sync_group, unsigned long long time, void * source);
		//void requestFrameSynchronization (int sync_group, unsigned long long frame, void * source);

	protected:
		LogConfig & m_config;
		logs::CtxLogConfig m_config_ui;
		QString m_fname;
		Connection * m_connection;
		QWidget * m_tab;

		SessionState m_session_state;
		bool m_column_setup_done;
		int m_last_search_row;
		int m_last_search_col;
		QString m_last_search;
		QString m_curr_preset;
		QWidget * m_tab_widget;
		QTableView * m_table_view_widget;
		TreeModel * m_file_model;
		TreeProxyModel * m_file_proxy;
		QItemSelectionModel * m_proxy_selection;
		QStandardItemModel * m_ctx_model;
		QStandardItemModel * m_func_model;
		QStandardItemModel * m_tid_model;
		QStandardItemModel * m_color_regex_model;
		QStandardItemModel * m_regex_model;
		QStandardItemModel * m_lvl_model;
		QStandardItemModel * m_string_model;

		enum E_Delegates {
			  e_delegate_Level
			, e_delegate_Ctx
			, e_delegate_String
			, e_delegate_Regex
			, e_delegate_max_enum_value
		};
		boost::tuple<LevelDelegate *, CtxDelegate *, StringDelegate *, RegexDelegate *> m_delegates;
		QAbstractProxyModel * m_table_view_proxy;
		QAbstractItemModel * m_table_view_src;

		QMenu m_ctx_menu;
		enum E_Actions {
			  e_action_ToggleRef
			, e_action_HidePrev
			, e_action_ExcludeFileLine
			, e_action_Find
			, e_action_Copy
			, e_action_ColorTag
			, e_action_Setup
			, e_action_max_enum_value
		};
		std::vector<QAction *> m_actions;
		QModelIndex m_last_clicked;

		QTextStream * m_file_csv_stream;


	};
}

