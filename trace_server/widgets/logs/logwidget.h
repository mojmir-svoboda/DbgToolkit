#pragma once
#include <QWidget>
#include "action.h"
#include "logconfig.h"
#include "logctxmenu.h"
#include "logselectionproxymodel.h"
#include <3rd/kde/klinkitemselectionmodel.h>
#include <widgets/warnimage.h>
#include "syncwidgets.h"
#include <widgets/buttoncache.h>
#include <dock/dock.h>
#include "logtableview.h"

class Connection;
class BaseProxyModel;
class ControlBarLog;
class ColorizeWidget;
struct ColorizeConfig;
class QuickStringWidget;

namespace logs {

	class FindProxyModel;
	struct LogTableModel;
	struct ExtLogFilterProxyModel;
	class LogHHeaderCtxMenu;

	class LogWidget : public QFrame, public DockedWidgetBase
	{
		Q_OBJECT
	public:
		enum { e_type = e_data_log };
		LogWidget (Connection * conn, LogConfig const & cfg, QString const & fname, QStringList const & path);
		virtual ~LogWidget ();

		virtual E_DataWidgetType type () const { return e_data_log; }
		virtual QWidget * controlWidget () { return 0; }
		LogConfig & config () { return m_config; }
		LogConfig const & config () const { return m_config; }
		void loadConfig (QString const & preset_dir);
		void loadAuxConfigs ();
		void saveConfig (QString const & preset_dir);
		void saveAuxConfigs ();
		void saveFindConfig ();
		void saveColorizeConfig ();
		void setModelAndConfig (LogConfig & pcfg);
		void reloadModelAccordingTo (LogConfig & pcfg);
		void applyConfig ();
		QString getCurrentWidgetPath () const;
		void fillButtonCache (QWidget * parent);
		void setButtonCache (ButtonCache * c) { m_cacheLayout = c; }
		void resizeSections ();

		std::vector<DecodedCommand> m_queue;
		void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
		void commitCommands (E_ReceiveMode mode);

		virtual bool handleAction (Action * a, E_ActionHandleType sync);
		virtual void setVisible (bool visible);
		void clearAllData ();

		E_SrcProtocol protocol () const;

		LogTableModel const * logModel () const { return m_src_model; }
		ExtLogFilterProxyModel const * logProxy () const { return m_proxy_model; }
		FindProxyModel const * findProxy () const { return m_find_proxy_model; }
		QModelIndex mapToSourceIndexIfProxy (QModelIndex const & idx) const;
		bool isModelProxy () const;
		void setupLogModel ();
		void setupRefsModel (LogTableModel * linked_model);
		void setupRefsProxyModel (LogTableModel * linked_model, BaseProxyModel * linked_proxy);
		void setupCloneModel (LogTableModel * src_model);
		void setupLogSelectionProxy ();

		proto::Record const * getRecord (QModelIndex const & row_index) const;
		proto::Record const * getRecord (int row) const;

		///virtual void scrollTo (QModelIndex const & index, ScrollHint hint = EnsureVisible);

	protected:
		friend struct LogTableModel;
		friend class MainWindow;
		friend class FilterProxyModel;
		friend class FindProxyModel;
		friend struct LogCtxMenu;
		friend class LogHHeaderCtxMenu;
		friend class LogDelegate;
		friend class LogWidget;
		friend class LogTableView;

		///virtual void wheelEvent (QWheelEvent * event);
		///virtual void keyPressEvent (QKeyEvent * event);
		QModelIndex currentSourceIndex () const;
		///virtual QModelIndex moveCursor (CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

		// config
		void defaultConfigFor (logs::LogConfig & config); // loads legacy registry defaults
		void reconfigureConfig (logs::LogConfig & cfg);
		void autoScrollOn ();
		void autoScrollOff ();

		// find & filtering
		bool filterEnabled () const { return m_config.m_filtering; }
		void setFilteringProxy (bool on);
		void setFindProxyModel (FindConfig const & fc);
		//void commitBatchToLinkedWidgets (int src_from, int src_to, BatchCmd const & batch);
		//void commitBatchToLinkedModel (int src_from, int src_to, BatchCmd const & batch);
		void setSrcModel (FindConfig const & fc);
		void handleFindAction (FindConfig const & fc);
		void handleQuickStringAction (QuickStringConfig const & fc);
		void handleColorizeAction (ColorizeConfig const & fc);
		//void findInWholeTable (FindConfig const & fc, QModelIndexList & result);
		LogWidget * mkFindAllRefsLogWidget (FindConfig const & fc);
		LogWidget * mkFindAllCloneLogWidget (FindConfig const & fc);
		void registerLinkedWidget (DockedWidgetBase * l);
		void unregisterLinkedWidget (DockedWidgetBase * l);
		void findAndSelect (FindConfig const & fc);
		//void findAndSelectNext (FindConfig const & fc);
		//void findAndSelectPrev (FindConfig const & fc);
		//void currSelection (QModelIndexList & sel) const;
		//void noMoreMatches ();
		void showWarningSign ();
		LogTableModel * cloneToNewModel (LogWidget * parent, FindConfig const & fc);
		LogTableModel * cloneToNewModelFromProxy (LogWidget * parent, BaseProxyModel * proxy, FindConfig const & fc);
		QString exportSelection ();

		// actions
		void addColorTagRow (int row);
		bool findColorTagRow (int row) const;
		void removeColorTagRow (int row);
		unsigned long long stimeRefValue () const { return m_stime_ref_value; }
		void clearRefSTime () { m_stime_ref_value = 0; }
		void setSTimeRefValue (unsigned long long t) { m_stime_ref_value = t; }
		unsigned long long ctimeRefValue() const { return m_ctime_ref_value; }
		void clearRefCTime() { m_ctime_ref_value = 0; }
		void setCTimeRefValue(unsigned long long t) { m_ctime_ref_value = t; }
		void findTableIndexInFilters (QModelIndex const & src_idx, bool scroll_to_item, bool expand);
		void excludeFile (QModelIndex const & row_index);
		void excludeSelectionRows (void (LogWidget::*fn)(QModelIndex const & idx));
		void excludeFileLine (QModelIndex const & row_index);
		void excludeRow (QModelIndex const & src_index);

		// filtering stuff
		FilterMgr * filterMgr () { return m_config_ui->m_ui->widget; }
		FilterMgr const * filterMgr () const { return m_config_ui->m_ui->widget; }
		ColorizerMgr * colorizerMgr () { return m_config_ui->m_ui->colorizer; }
		ColorizerMgr const * colorizerMgr () const { return m_config_ui->m_ui->colorizer; }
		SoundMgr * soundMgr () { return m_config_ui->m_ui->sound; }
		SoundMgr const * soundMgr () const { return m_config_ui->m_ui->sound; }

		void syncSelection (QModelIndexList const & sel);
		void clearFilters (QStandardItem * node);
		void clearFilters ();
		bool processByColorizers (QModelIndex const & sourceIndex);
		bool processBySounds (QModelIndex const & sourceIndex);
		void refreshFilters (BaseProxyModel const * proxy);
		void colorRow (int);

		// find
		void selectionFromTo (int & from, int & to) const;

		void scrollToCurrentTag ();
		void scrollToCurrentSelection ();
		void scrollToCurrentTagOrSelection ();
		void nextToView ();
		void emitRequestSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source) { emit requestSynchronization(mode, sync_group, time, this); }

		void linkToSource (LogWidget * src);
		bool isLinkedWidget () const { return m_linked_parent != 0; }

	public slots:
		void onShow ();
		void onHide ();
		void onHideContextMenu ();
		void onWindowActionTriggered ();
		void onShowContextMenu (QPoint const & pos);
		void onShowHHeaderContextMenu (QPoint const & pos);
		void setConfigValuesToUI (LogConfig const & cfg);
		void setUIValuesToConfig (LogConfig & cfg);
		void onCtxMenuAutoScrollStateChanged (int state);
		void onCtxMenuShowScopesChanged (int value);
		void onCtxMenuShowdtScopesChanged (int value);
		void onCtxMenuIndentChanged (int value);
		void onCtxMenuCutPathChanged (int state);
		void onCtxMenuCutPathLevelChanged (int value);
		void onCtxMenuCutNamespaceChanged (int value);
		void onCtxMenuCutNamespaceLevelChanged (int value);
		void onCtxMenuTableRowSizeChanged (int value);
		void onCtxMenuSyncGroupChanged (int value);
		void onSaveButton ();
		void onApplyButton ();
		void onResetButton ();
		void onDefaultButton ();
		void onRefillFilters ();
		void onFind ();
		void onFindNext ();
		void onFindPrev ();
		void onFindAllRefs ();
		void onQuickString ();
		void onPopAction ();

		void onColorize ();
		void onColorizeNext ();
		void onColorizePrev ();

		void performSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source);

		void onSectionResized (int logicalIndex, int oldSize, int newSize);
		void onSectionMoved (int logical, int old_visual, int new_visual);

		void onNextToView ();
		void onFilterChanged ();

		void onInvalidateFilter ();
		void onFilterEnabledChanged ();
		void onTableFontToolButton ();

		void onDumpFilters ();
		void onTableClicked (QModelIndex const & row_index);
		void onTableDoubleClicked (QModelIndex const & row_index);
		void onSyncGroupValueChanged (int i);
		void onSyncGroupRowButton ();

		// actions
		/*void onClearCurrentView ();*/
		void onClearAllDataButton ();
		/*void onUnhidePrevFromRow ();*/
		void exportStorageToCSV (QString const & path);
		void onCopyToClipboard ();
		void onExcludeFile ();
		void onExcludeFileLine ();
		void onGotoPrevErr();
		void onGotoNextErr();
		void onGotoPrevWarn();
		void onGotoNextWarn();
		void onExcludeRow ();
		void onLocateRow ();
		void onColorFileLine ();
		void onColorRow ();
		void onColorLastRow ();
		void onGotoPrevColor();
		void onGotoNextColor();
		void onUncolorRow ();
		void onSetRefCTime ();
		void onSetRefSTime ();
		void onHidePrev ();
		void onHideNext ();
		void onChangeTimeUnits (int);
		void onOpenFileLine ();

		/*void requestTableWheelEventSync (QWheelEvent * ev, QTableView const * source);
		void requestTableActionSync (unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QTableView const * source);*/
		void findNearestRow4Time (bool ctime, unsigned long long t);

	signals:
		void requestSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source);

	protected:
		Connection * m_connection;
		LogTableView * m_tableview;
		ButtonCache * m_cacheLayout; // button cache
		QToolButton * m_gotoPrevErrButton;
		QToolButton * m_gotoNextErrButton;
		QToolButton * m_gotoPrevWarnButton;
		QToolButton * m_gotoNextWarnButton;
		QToolButton * m_openFileLineButton;
		QToolButton * m_excludeFileButton;
		QToolButton * m_excludeFileLineButton;
		QToolButton * m_excludeRowButton;
		QToolButton * m_locateRowButton;
		QSpinBox * m_syncGroup;
		QToolButton * m_syncGroupRowButton;
		QToolButton * m_clrDataButton;
		TimeComboBox * m_timeComboBox;
		QToolButton * m_setRefCTimeButton;
		QToolButton * m_setRefSTimeButton;
		QToolButton * m_hidePrevButton;
		QToolButton * m_hideNextButton;
		QToolButton * m_colorRowButton;
		QToolButton * m_colorFileLineButton;
		QToolButton * m_uncolorRowButton;
		QToolButton * m_gotoPrevColorButton;
		QToolButton * m_gotoNextColorButton;
		ControlBarLog * m_control_bar;

		LogConfig m_config;
		LogCtxMenu * m_config_ui;
		LogHHeaderCtxMenu * m_config_hheader_ui;
		QString m_fname;
		WarnImage * m_warnimage;
		unsigned long long m_ctime_ref_value;
		unsigned long long m_stime_ref_value;
		ExtLogFilterProxyModel * m_proxy_model;
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
		QuickStringWidget * m_quick_string_widget;
		ColorizeWidget * m_colorize_widget;
		QAction * m_window_action;
		LogWidget * m_linked_parent; // @TODO: move to LogWidgetWithButtons
		typedef std::vector<DockedWidgetBase *> linked_widgets_t;
		linked_widgets_t m_linked_widgets;
		QTextStream * m_file_csv_stream;
	};

	void fillDefaultConfigWithLogTags (CheckedComboBoxConfig & cccfg);
}

