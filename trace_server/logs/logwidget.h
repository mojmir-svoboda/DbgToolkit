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
#include "syncwidgets.h"
#include "buttoncache.h"

class Connection;
class LogTableModel;
class FindProxyModel;
class BaseProxyModel;

namespace logs {

	class LogWidget : public TableView, public ActionAble
	{
		Q_OBJECT
	public:
		LogWidget (Connection * conn, QWidget * wparent, LogConfig & cfg, QString const & fname, QStringList const & path);

		virtual ~LogWidget ();

		LogConfig & getConfig () { return m_config; }
		LogConfig const & getConfig () const { return m_config; }
		void loadConfig (QString const & preset_dir);
		void loadAuxConfigs ();
		void saveConfig (QString const & preset_dir);
		void saveAuxConfigs ();
		void saveFindConfig ();
		void resizeModelToConfig (LogConfig & pcfg);
		void reloadModelAccordingTo (LogConfig & pcfg);
		void applyConfig ();
		QString getCurrentWidgetPath () const;
		void fillButtonCache ();
		void setButtonCache (ButtonCache * c) { m_cacheLayout = c; }

		QList<DecodedCommand> m_queue;
		void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
		void commitCommands (E_ReceiveMode mode);

		virtual bool handleAction (Action * a, E_ActionHandleType sync);

		int findColumn4Tag (tlv::tag_t tag);
		int findColumn4TagCst (tlv::tag_t tag) const;

		LogTableModel const * logModel () const { return m_src_model; }
		FilterProxyModel const * logProxy () const { return m_proxy_model; }
		bool isModelProxy () const;
		void setupLogModel (LogTableModel * src_model);
		void setupNewLogModel ();
		void setupLogSelectionProxy ();
		void linkToSource (DockedWidgetBase * src);
		bool isLinkedWidget () const { return m_linked_parent != 0; }

		DecodedCommand const * getDecodedCommand (QModelIndex const & row_index);
		DecodedCommand const * getDecodedCommand (int row);

		//FindWidget * findWidget () { return m_config_ui.m_ui->findWidget; }
		//FindWidget const * findWidget () const { return m_config_ui.m_ui->findWidget; }

		void setDockedWidget (DockedWidgetBase * dwb) { m_dwb = dwb; }

		virtual void scrollTo (QModelIndex const & index, ScrollHint hint = EnsureVisible);

	protected:
		friend class LogTableModel;
		friend class FilterProxyModel;
		friend class FindProxyModel;
		friend struct LogCtxMenu;
		friend struct LogDelegate;

		virtual void wheelEvent (QWheelEvent * event);
		virtual void keyPressEvent (QKeyEvent * event);
		virtual QModelIndex moveCursor (CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

		// config
		int sizeHintForColumn (int column) const;
		void defaultConfigFor (logs::LogConfig & config); // loads legacy registry defaults
		void reconfigureConfig (logs::LogConfig & cfg);
		void normalizeConfig (logs::LogConfig & normalized);
		void swapSectionsAccordingTo (logs::LogConfig const & cfg);
		void resizeSections ();
		void autoScrollOn ();
		void autoScrollOff ();

		// find & filtering
		bool filterEnabled () const { return m_config.m_filtering; }
		void setFilteringProxy (bool on);
		void setFindProxyModel (FindConfig const & fc);
		void setSrcModel (FindConfig const & fc);
		void handleFindAction (FindConfig const & fc);
		void findInWholeTable (FindConfig const & fc, QModelIndexList & result);
		LogWidget * mkFindAllRefsLogWidget (FindConfig const & fc);
		LogWidget * mkFindAllCloneLogWidget (FindConfig const & fc);
		void registerLinkedWidget (DockedWidgetBase * l);
		void unregisterLinkedWidget (DockedWidgetBase * l);
		void findAndSelect (FindConfig const & fc);
		void findAndSelectNext (FindConfig const & fc);
		void findAndSelectPrev (FindConfig const & fc);
		void currSelection (QModelIndexList & sel) const;
		void noMoreMatches ();
		LogTableModel * cloneToNewModel (FindConfig const & fc);
		QString exportSelection ();

		// actions
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
		ColorizerMgr * colorizerMgr () { return m_config_ui.m_ui->colorizer; }
		ColorizerMgr const * colorizerMgr () const { return m_config_ui.m_ui->colorizer; }
		void syncSelection (QModelIndexList const & sel);
		void clearFilters (QStandardItem * node);
		void clearFilters ();
		bool appendToFilters (DecodedCommand const & cmd);
		bool appendToColorizers (DecodedCommand const & cmd);
		void appendToFileLineFilters (QString const & item);
		void appendToTIDFilters (QString const & item);
		//void appendToLvlWidgets (FilteredLevel const & flt);
		void appendToLvlFilters (QString const & item);
		//void appendToCtxWidgets (FilteredContext const & flt);
		void appendToCtxFilters (QString const & item, bool checked);
		void appendToRegexFilters (QString const & str, bool checked, bool inclusive);
		void removeFromRegexFilters (QString const & val);
		void appendToStringFilters (QString const & str, bool checked, int state);
		void removeFromStringFilters (QString const & val);
		void refreshFilters (BaseProxyModel const * proxy);

		void setRefFromRow ();
		void clearRefTime () { m_time_ref_row = 0; }
		void colorRow (int);

		/*void removeFromColorRegex (QString const & val);
		//void loadToColorRegexps (QString const & filter_item, QString const & color, bool enabled);
		void onColorRegexChanged (int role);
		void recompileColorRegexps ();
		void recompileColorRegex (ColorizedText & ct);
		void actionColorRegex (DecodedCommand const & cmd, ColorizedText const & ct) const;
		void actionUncolorRegex (DecodedCommand const & cmd, ColorizedText const & ct) const;
		void updateColorRegex (ColorizedText const & ct);
		void uncolorRegex (ColorizedText const & ct);*/

		//void loadToRegexps (QString const & filter_item, bool inclusive, bool enabled);

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
		void onRefillFilters ();
		void onFind ();
		void onFindNext ();
		void onFindPrev ();
		void onFindAllRefs ();

		//void scrollTo (QModelIndex const & index, ScrollHint hint);
		void performSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source);

		void onClearAllDataButton ();
		void onSectionResized (int logicalIndex, int oldSize, int newSize);
		void onSectionMoved (int logical, int old_visual, int new_visual);

		void onNextToView ();
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
		//void onUnhidePrevFromRow ();
		void exportStorageToCSV (QString const & filename);
		void onCopyToClipboard ();
		void onExcludeFileLine ();
		void onGotoPrevErr();
		void onGotoNextErr();
		void onGotoPrevWarn();
		void onGotoNextWarn();
		void onExcludeRow ();
		void onLocateRow ();
		void onColorFileLine ();
		void onColorRow ();
		void onGotoPrevColor();
		void onGotoNextColor();
		void onUncolorRow ();
		void onSetRefTime ();
		void onHidePrev ();
		void onHideNext ();
		void onChangeTimeUnits ();

		//void requestTableWheelEventSync (QWheelEvent * ev, QTableView const * source);
		//void requestTableActionSync (unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QTableView const * source);
		void findNearestRow4Time (bool ctime, unsigned long long t);

	signals:
		void requestSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source);

	protected:
		Connection * m_connection;
		ButtonCache * m_cacheLayout;

		QToolButton * m_gotoPrevErrButton;
		QToolButton * m_gotoNextErrButton;
		QToolButton * m_gotoPrevWarnButton;
		QToolButton * m_gotoNextWarnButton;
		QToolButton * m_excludeFileLineButton;
		QToolButton * m_excludeRowButton;
		QToolButton * m_locateRowButton;
		TimeComboBox * m_timeComboBox;
		QToolButton * m_setRefTimeButton;
		QToolButton * m_hidePrevButton;
		QToolButton * m_hideNextButton;
		QToolButton * m_colorRowButton;
		QToolButton * m_colorFileLineButton;
		QToolButton * m_uncolorRowButton;
		QToolButton * m_gotoPrevColorButton;
		QToolButton * m_gotoNextColorButton;

		LogConfig & m_config;
		LogConfig m_config2;
		logs::LogCtxMenu m_config_ui;
		QString m_fname;
		QWidget * m_tab;
		DockedWidgetBase * m_linked_parent;
		WarnImage * m_warnimage;
		DockedWidgetBase * m_dwb;
		typedef std::vector<DockedWidgetBase *> linked_widgets_t;
		linked_widgets_t m_linked_widgets;

		FilterState m_filter_state;

		// mutable state
		TagConfig m_tagconfig;
		QMap<tlv::tag_t, int> m_tags2columns;
		ThreadSpecific m_tls;

		int m_time_ref_row;
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
		QStandardItemModel * m_color_regex_model;
		FindWidget * m_find_widget;

		QModelIndex m_last_clicked;
		QString m_csv_separator;
		QTextStream * m_file_csv_stream;
	};

	class LogWidgetWithButtons : public QWidget, public ActionAble
	{
		Q_OBJECT
	public:

		LogWidget * m_lw;

		LogWidgetWithButtons (Connection * conn, QWidget * wparent, LogConfig & cfg, QString const & fname, QStringList const & path);
		~LogWidgetWithButtons ();

		virtual bool handleAction (Action * a, E_ActionHandleType sync)
		{
			return m_lw->handleAction(a, sync);
		}

		void loadConfig (QString const & preset_dir) { m_lw->loadConfig(preset_dir); }
		void saveConfig (QString const & preset_dir) { m_lw->saveConfig(preset_dir); }
		void applyConfig () { m_lw->applyConfig(); }

		void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode) { m_lw->handleCommand(cmd, mode); }
		void exportStorageToCSV (QString const & filename) { m_lw->exportStorageToCSV(filename); }
		void commitCommands (E_ReceiveMode mode) { m_lw->commitCommands(mode); }
		void setupNewLogModel () { m_lw->setupNewLogModel(); }

		//FindWidget * findWidget () { return m_lw->findWidget(); }
		//FindWidget const * findWidget () const { return m_lw->findWidget(); }

		void setDockedWidget (DockedWidgetBase * dwb) { m_lw->setDockedWidget(dwb); }

	public slots:
		void onHideContextMenu () { m_lw->onHideContextMenu(); }
	};
}

