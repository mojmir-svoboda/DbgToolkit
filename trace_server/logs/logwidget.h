#pragma once
#include <QWidget>
#include <tableview.h>
#include <cmd.h>
#include <QStyledItemDelegate>
#include <QItemDelegate>
#include "action.h"
#include "logconfig.h"
#include "logctxmenu.h"
#include "tagconfig.h"
#include "logselectionproxymodel.h"
#include <kde/klinkitemselectionmodel.h>
#include <appdata.h>
#include "warnimage.h"

class Connection;
class LogTableModel;
class FindProxyModel;

namespace logs {

	class LogWidget : public TableView, public ActionAble
	{
		Q_OBJECT
	public:
		LogWidget (Connection * oparent, QWidget * wparent, LogConfig & cfg, QString const & fname, QStringList const & path);

		virtual ~LogWidget ();

		LogConfig & getConfig () { return m_config; }
		LogConfig const & getConfig () const { return m_config; }
		void loadConfig (QString const & preset_dir);
		void loadAuxConfigs ();
		void saveConfig (QString const & preset_dir);
		void saveAuxConfigs ();
		void saveFindConfig ();
		void applyConfig (LogConfig & pcfg);
		void applyConfig ();
		QString getCurrentWidgetPath () const;

		QList<DecodedCommand> m_queue;
		void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
		void commitCommands (E_ReceiveMode mode);

		virtual bool handleAction (Action * a, E_ActionHandleType sync);

		int findColumn4Tag (tlv::tag_t tag);

		LogTableModel const * logModel () const { return m_src_model; }
		FilterProxyModel const * logProxy () const { return m_proxy_model; }
		bool isModelProxy () const;
		void setupLogModel (LogTableModel * src_model);
		void setupNewLogModel ();
		void setupLogSelectionProxy ();
		void linkToSource (LogWidget * src);
		bool isLinkedWidget () const { return m_linked_parent != 0; }

		DecodedCommand const * getDecodedCommand (QModelIndex const & row_index);
		DecodedCommand const * getDecodedCommand (int row);

	protected:
		friend class LogTableModel;
		friend class FilterProxyModel;
		friend class FindProxyModel;
		friend struct LogCtxMenu;
		friend struct LogDelegate;

		virtual void keyPressEvent (QKeyEvent * event);

		// config
		int sizeHintForColumn (int column) const;
		void defaultConfigFor (logs::LogConfig & config); // loads legacy registry defaults
		void reconfigureConfig (logs::LogConfig & cfg);
		bool convertBloodyBollockyBuggeryRegistry (logs::LogConfig & cfg);
		void normalizeConfig (logs::LogConfig & normalized);
		void swapSectionsAccordingTo (logs::LogConfig const & cfg);
		void resizeSections ();

		// find & filtering
		bool filterEnabled () const { return m_config.m_filtering; }
		void setFilteringProxy (bool on);
		void setFindProxyModel (FindConfig const & fc);
		void setSrcModel (FindConfig const & fc);
		void handleFindAction (FindConfig const & fc);
		void findInWholeTable (FindConfig const & fc, QModelIndexList & result);
		LogWidget * mkFindAllRefsLogWidget (FindConfig const & fc);
		LogWidget * mkFindAllCloneLogWidget (FindConfig const & fc);
		void registerLinkedLog (LogWidget * l);
		void unregisterLinkedLog (LogWidget * l);
		void findAndSelect (FindConfig const & fc);
		void findAndSelectNext (FindConfig const & fc);
		void findAndSelectPrev (FindConfig const & fc);
		void currSelection (QModelIndexList & sel) const;
		void noMoreMatches ();
		LogTableModel * cloneToNewModel (FindConfig const & fc);
		QString exportSelection ();

		// actions
		void excludeContentToRow (int row) { m_exclude_content_to_row = row; }
		int excludeContentToRow () const { return m_exclude_content_to_row; }
		void setTimeRefFromRow (int row) { m_time_ref_row = row; }
		int timeRefFromRow () const { return m_time_ref_row; }
		void addColorTagRow (int row);
		bool findColorTagRow (int row) const;
		void removeColorTagRow (int row);
		unsigned long long timeRefValue () const { return m_time_ref_value; }
		void setTimeRefValue (unsigned long long t) { m_time_ref_value = t; }
		void findTableIndexInFilters (QModelIndex const & src_idx, bool scroll_to_item, bool expand);
		void excludeFileLine (QModelIndex const & row_index);
		void excludeRow (QModelIndex const & src_index);

		// filtering stuff
		FilterMgr * filterMgr () { return m_config_ui.m_ui->widget; }
		FilterMgr const * filterMgr () const { return m_config_ui.m_ui->widget; }
		void syncSelection (QModelIndexList const & sel);
		void clearFilters (QStandardItem * node);
		void clearFilters ();
		void appendToTIDFilters (QString const & item);
		void appendToLvlWidgets (FilteredLevel const & flt);
		void appendToLvlFilters (QString const & item);
		void appendToCtxWidgets (FilteredContext const & flt);
		void appendToCtxFilters (QString const & item, bool checked);
		bool appendToFilters (DecodedCommand const & cmd);
		void appendToRegexFilters (QString const & str, bool checked, bool inclusive);
		void removeFromRegexFilters (QString const & val);
		void appendToStringFilters (QString const & str, bool checked, int state);
		void removeFromStringFilters (QString const & val);

		void setRefFromRow ();
		void clearRefTime () { m_time_ref_row = 0; }
		void colorRow (int);
	
	void setupSeparatorChar (QString const & c);
	QString separatorChar () const;

	// find
	void selectionFromTo (int & from, int & to) const;
	QString findString4Tag (tlv::tag_t tag, QModelIndex const & row_index) const;
	QVariant findVariant4Tag (tlv::tag_t tag, QModelIndex const & row_index) const;

	void scrollToCurrentTag ();
	void scrollToCurrentSelection ();
	void scrollToCurrentTagOrSelection ();
	void nextToView ();
	void onFindFileLine (QModelIndex const &);

	void appendToColorRegexFilters (QString const & val);
	void removeFromColorRegexFilters (QString const & val);
	void loadToColorRegexps (QString const & filter_item, QString const & color, bool enabled);
	void onColorRegexChanged ();
	void recompileColorRegexps ();
	//void loadToRegexps (QString const & filter_item, bool inclusive, bool enabled);

	FilterState & filterState () { return m_filter_state; }
	FilterState const & filterState () const { return m_filter_state; }

	int findColumn4TagInTemplate (tlv::tag_t tag) const;
	int appendColumn (tlv::tag_t tag);

	ThreadSpecific & getTLS () { return m_tls; }
	ThreadSpecific const & getTLS () const { return m_tls; }

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
		void onSectionMoved (int logical, int old_visual, int new_visual);

		void onNextToView ();
		void turnOffAutoScroll ();
		void onAutoScrollHotkey ();
		void onFilterChanged ();
		//void onFilterFile (int state);

		void onInvalidateFilter ();
		void onFilterEnabledChanged ();
		void onTableFontToolButton ();

		void onDumpFilters ();
		void onTableClicked (QModelIndex const & row_index);
		void onTableDoubleClicked (QModelIndex const & row_index);

		// actions
		void onClearCurrentView ();
		void onHidePrevFromRow ();
		//void onUnhidePrevFromRow ();
		void exportStorageToCSV (QString const & filename);
		void onCopyToClipboard ();
		void onExcludeFileLine ();
		void onExcludeRow ();
		void onLocateRow ();
		void onColorFileLine ();
		void onColorRow ();
		void onUncolorRow ();
		void onSetRefTime ();
		void onHidePrev ();
		void onHideNext ();




	signals:
		//void requestTimeSynchronization (int sync_group, unsigned long long time, void * source);
		//void requestFrameSynchronization (int sync_group, unsigned long long frame, void * source);

	protected:
		Connection * m_connection;
		LogConfig & m_config;
		LogConfig m_config2;
		logs::LogCtxMenu m_config_ui;
		QString m_fname;
		QWidget * m_tab;
		LogWidget * m_linked_parent;
		WarnImage * m_warnimage;
		std::vector<LogWidget *> m_linked_widgets;

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


		FilterProxyModel * m_proxy_model;
		FindProxyModel * m_find_proxy_model;
		LogTableModel * m_src_model;
		LogSelectionProxyModel * m_selection;
		KLinkItemSelectionModel * m_ksrc_selection;
		KLinkItemSelectionModel * m_kproxy_selection;
		QItemSelectionModel * m_src_selection;
		QItemSelectionModel * m_proxy_selection;
		QItemSelectionModel * m_find_proxy_selection;
		QItemSelectionModel * m_kfind_proxy_selection;

		QModelIndex m_last_clicked;

		QString m_csv_separator;
		QTextStream * m_file_csv_stream;
	};

}

