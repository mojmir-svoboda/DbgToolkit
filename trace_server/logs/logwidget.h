#pragma once
#include <QWidget>
#include <tableview.h>
#include <cmd.h>
#include "logconfig.h"
#include "logctxmenu.h"
#include "tagconfig.h"

class Connection;
class LevelDelegate;
class CtxDelegate;
class StringDelegate;
class RegexDelegate;
class LogTableModel;

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

		void loadConfig (QString const & path);
		void saveConfig (QString const & path);


		void appendCommand (DecodedCommand const & cmd);

	protected:
		friend class LogTableModel;
		friend class FilterProxyModel;

		bool filterEnabled () const { return m_config.m_filtering; }

	void setupSeparatorChar (QString const & c);
	QString separatorChar () const;
	void onSetup (E_SrcProtocol const proto, int curr_app_idx = -1, bool first_time = false, bool mac_user = false);
	//void onSetupCSV (int curr_app_idx = -1, bool first_time = false, bool mac_user = false);
	//void onSetupCSVSeparator (int curr_app_idx = -1, int column = -1, bool first_time = false);
	void onSetupCSVColumns (int curr_app_idx, int columns, bool first_time);
	void onSetupCSVSeparator (int curr_app_idx, bool first_time);
	void onSettingsAppSelectedTLV (int idx, bool first_time = false);
	void onSettingsAppSelectedCSV (int idx, int columns, bool first_time = false);
	void settingsDisableButSeparator ();
	void settingsToDefault ();
	void onClickedAtSettingPooftahButton ();
	void onClickedAtSettingOkButton ();
	void onClickedAtSettingOkSaveButton ();
	void onClickedAtSettingCancelButton ();
	void onClickedAtSettingColumnSetup (QModelIndex idx);
	void onClickedAtSettingColumnSizes (QModelIndex idx);
	void onClickedAtSettingColumnAlign (QModelIndex idx);
	void onClickedAtSettingColumnElide (QModelIndex idx);
	// setup
	void onSetupAction ();
	void onListVisibilityChanged (bool);
	void prepareSettingsWidgets (int idx, bool first_time = false);
	void clearSettingWidgets ();




// divny
//bool filterEnabled () const { return m_config_ui->ui()->filterFileCheckBox->isEnabled() && m_config_ui->ui()->filterFileCheckBox->isChecked(); }

	// find
	void findTextInAllColumns (QString const & text, int from_row, int to_row, bool only_first);
	bool matchTextInCell (QString const & text, int row, int col);
	void endOfSearch ();
	void findTextInColumn (QString const & text, int col, int from_row, int to_row);
	void findTextInColumnRev (QString const & text, int col, int from_row, int to_row);
	void selectionFromTo (int & from, int & to) const;
	void findAllTexts (QString const & text);
	void findText (QString const & text, tlv::tag_t tag);
	void findText (QString const & text);
	void findNext (QString const & text);
	void findPrev (QString const & text);
	QString findString4Tag (tlv::tag_t tag, QModelIndex const & row_index) const;
	QVariant findVariant4Tag (tlv::tag_t tag, QModelIndex const & row_index) const;
	void scrollToCurrentTag ();
	void scrollToCurrentSelection ();
	void scrollToCurrentTagOrSelection ();
	void nextToView ();
	void onFindFileLine (QModelIndex const &);

	// filtering stuff
	void onInvalidateFilter ();
	void syncSelection (QModelIndexList const & sel);
	void setFilterFile (int state);
	void clearFilters (QStandardItem * node);
	void clearFilters ();
	void onClearCurrentFileFilter ();
	void onClearCurrentCtxFilter ();
	void onClearCurrentTIDFilter ();
	void onClearCurrentColorizedRegexFilter ();
	void onClearCurrentRegexFilter ();
	void onClearCurrentStringFilter ();
	void onClearCurrentScopeFilter ();
	void onClearCurrentRefTime ();
	void onExcludeFileLine (QModelIndex const & row_index);
	void onFileColOrExp (QModelIndex const & idx, bool collapsed);
	void onFileExpanded (QModelIndex const & idx);
	void onFileCollapsed (QModelIndex const & idx);
	void appendToTIDFilters (QString const & item);
	void appendToLvlWidgets (FilteredLevel const & flt);
	void appendToLvlFilters (QString const & item);
	void appendToCtxWidgets (FilteredContext const & flt);
	void appendToCtxFilters (QString const & item, bool checked);
	bool appendToFilters (DecodedCommand const & cmd);
	void appendToRegexFilters (QString const & str, bool checked, bool inclusive);
	void removeFromRegexFilters (QString const & val);
	void recompileRegexps ();
	void appendToStringWidgets (FilteredString const & flt);
	void appendToStringFilters (QString const & str, bool checked, int state);
	void removeFromStringFilters (QString const & val);
	void recompileStrings ();
	void appendToColorRegexFilters (QString const & val);
	void removeFromColorRegexFilters (QString const & val);
	void loadToColorRegexps (QString const & filter_item, QString const & color, bool enabled);
	void onColorRegexChanged ();
	void recompileColorRegexps ();
	void loadToRegexps (QString const & filter_item, bool inclusive, bool enabled);
	void onFilterFileComboChanged (QString str);
	void onCancelFilterFileButton ();
	void onCutParentValueChanged (int i);
	void onCollapseChilds ();


	FilterState & filterState () { return m_filter_state; }
	FilterState const & filterState () const { return m_filter_state; }


	QAbstractTableModel const * modelView () const { return static_cast<QAbstractTableModel const *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : model()); }
	QAbstractProxyModel const * proxyView () const { return static_cast<QAbstractProxyModel const *>(m_table_view_proxy); }

	bool isModelProxy () const;

	int findColumn4TagInTemplate (tlv::tag_t tag) const;
	int findColumn4Tag (tlv::tag_t tag) const;
	int appendColumn (tlv::tag_t tag);

	ThreadSpecific & getTLS () { return m_tls; }
	ThreadSpecific const & getTLS () const { return m_tls; }

	FilterWidget * filterWidget () { return m_config_ui.m_ui->widget; }
	FilterWidget const * filterWidget () const { return m_config_ui.m_ui->widget; }





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

		void onNextToView ();
		void turnOffAutoScroll ();
		void onAutoScrollHotkey ();
		void onFilterFile (int state);
		void onTableFontToolButton ();
		void onEditFind ();
		void onQSearch (QString const & text);
		void onQSearchEditingFinished ();
		void setLastSearchIntoCombobox (QString const & txt);
		void onFindAllButton ();
		void onQFilterLineEditFinished ();
		void appendToSearchHistory (QString const & str);
		void updateSearchHistory ();
		void onEditFindNext ();
		void onEditFindPrev ();
		void onDumpFilters ();
		void appendLog (DecodedCommand const & cmd);
		void applyConfig ();
		void onTableClicked (QModelIndex const & row_index);
		void onTableDoubleClicked (QModelIndex const & row_index);
		void onApplyColumnSetup ();
		void onExcludeFileLine ();
		void onToggleRefFromRow ();
		void onColorTagRow (int);
		void onClearCurrentView ();
		void onHidePrevFromRow ();
		void onUnhidePrevFromRow ();
		void exportStorageToCSV (QString const & filename);

		//void setupColumns (QList<QString> * cs_template, columns_sizes_t * sizes , columns_align_t * ca_template, columns_elide_t * ce_template);
		//void setupColumnsCSV (QList<QString> * cs_template, columns_sizes_t * sizes , columns_align_t * ca_template, columns_elide_t * ce_template);
		QString onCopyToClipboard ();
		void findTableIndexInFilters (QModelIndex const & src_idx, bool scroll_to_item, bool expand);


	void excludeContentToRow (int row) { m_exclude_content_to_row = row; }
	int excludeContentToRow () const { return m_exclude_content_to_row; }
	void setTimeRefFromRow (int row) { m_time_ref_row = row; }
	int timeRefFromRow () const { return m_time_ref_row; }
	void addColorTagRow (int row);
	bool findColorTagRow (int row) const;
	void removeColorTagRow (int row);
	unsigned long long timeRefValue () const { return m_time_ref_value; }
	void setTimeRefValue (unsigned long long t) { m_time_ref_value = t; }
	void onClearRefTime () { m_time_ref_row = 0; }


	signals:
		//void requestTimeSynchronization (int sync_group, unsigned long long time, void * source);
		//void requestFrameSynchronization (int sync_group, unsigned long long frame, void * source);

	protected:
		LogConfig m_config;
		logs::CtxLogConfig m_config_ui;
		QString m_fname;
		Connection * m_connection;
		QWidget * m_tab;

		FilterState m_filter_state;

		// mutable state
		TagConfig m_tagconfig;
		QMap<tlv::tag_t, int> m_tags2columns;
		ThreadSpecific m_tls;

		int m_last_search_row;
		int m_last_search_col;
		QString m_last_search;
		bool m_column_setup_done;

		int m_exclude_content_to_row;
		int m_time_ref_row;
		QVector<int> m_color_tag_rows;
		int m_current_tag;
		int m_current_selection;
		unsigned long long m_time_ref_value;


		QString m_curr_preset;
		QTableView * m_table_view_widget;

		QAbstractProxyModel * m_table_view_proxy;
		LogTableModel * m_table_view_src;

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

		QString m_csv_separator;
		QTextStream * m_file_csv_stream;
	};
}

