#pragma once
#include <QWidget>
#include <QColorDialog>
#include <QTableView>
#include <QAbstractItemView>
#include "tableconfig.h"
#include "tablectxmenu.h"
#include "tablemodel.h"
#include <action.h>
#include "baseproxymodel.h"
#include <dock.h>
#include <syncwidgets.h>
#include <cmd.h>
#include <findwidget.h>
#include <tableview.h>
#include <warnimage.h>

class Connection;

namespace table {

	class TableWidget : public TableView, public DockedWidgetBase
	{
		Q_OBJECT
	public:
		enum { e_type = e_data_table };
		TableWidget (Connection * conn, TableConfig const & cfg, QString const & fname, QStringList const & path);

		virtual E_DataWidgetType type () const { return e_data_table; }
		virtual QWidget * controlWidget () { return 0; }
		void applyConfig (TableConfig & pcfg);
		virtual ~TableWidget ();

		TableConfig & config () { return m_config; }
		TableConfig const & config () const { return m_config; }

		void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
		void commitCommands (E_ReceiveMode mode);
		virtual bool handleAction (Action * a, E_ActionHandleType sync);
		virtual void setVisible (bool visible);

		void appendTableXY (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & msg);
		void appendTableSetup (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & hhdr, QString const & tag);

		bool isModelProxy () const;
		void onInvalidateFilter ();
		void findNearestTimeRow (unsigned long long t);

		//void requestTableWheelEventSync (QWheelEvent * ev, QTableView const * source);
		//void requestTableActionSync (unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QTableView const * source);

		QString getCurrentWidgetPath () const;
		void loadAuxConfigs ();
		void saveAuxConfigs ();
		void saveFindConfig ();
		void loadConfig (QString const & path);
		void saveConfig (QString const & path);
		void applyConfig ();
		void exportStorageToCSV (QString const & filename) { }
		void autoScrollOff ();
		void autoScrollOn ();
		QModelIndex currentSourceIndex () const;

		void findNearestRow4Time (bool ctime, unsigned long long t);

		FilterMgr * filterMgr () { return m_config_ui.m_ui->widget; }
		FilterMgr const * filterMgr () const { return m_config_ui.m_ui->widget; }
		ColorizerMgr * colorizerMgr () { return m_config_ui.m_ui->colorizer; }
		ColorizerMgr const * colorizerMgr () const { return m_config_ui.m_ui->colorizer; }

	protected:
		virtual void wheelEvent (QWheelEvent * event);
    virtual void keyPressEvent (QKeyEvent * event);
		virtual QModelIndex	moveCursor (CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

    virtual void showWarningSign ();

	public Q_SLOTS:

		void onShow ();
		void onHide ();
		void onHideContextMenu ();
		void onShowContextMenu (QPoint const & pos);
		void setConfigValuesToUI (TableConfig const & cfg);
		void setUIValuesToConfig (TableConfig & cfg);
		void filteringStateChanged (int);
		void onSaveButton ();
		void onApplyButton ();
		void onResetButton ();
		void onDefaultButton ();
		void onSectionResized (int idx, int old_size, int new_size);
		void scrollTo (QModelIndex const & index, ScrollHint hint);
		void onTableDoubleClicked (QModelIndex const & row_index);
		void onClickedAtColumnSetup (QModelIndex const idx);
		void onCtxMenuAutoScrollStateChanged (int state);
		void onCtxMenuSparseStateChanged (int state);
		void onCtxMenuSyncGroupChanged (int value);
		void onClearAllData ();
		void performSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source);
		void handleFindAction (FindConfig const & fc);
		void onFind();
		void onFindNext();
		void onFindPrev();
		void onFindAllRefs();

	signals:
		void requestSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source);

	protected:
		TableConfig m_config;
		table::CtxTableConfig m_config_ui;
		//QList<QColor> m_colors;
		QString m_fname;
		FindWidget * m_find_widget;
		WarnImage * m_warnimage;
		//ColorizeWidget * m_colorize_widget;
		TableModel * m_src_model;
		BaseProxyModel * m_proxy_model;
		Connection * m_connection;
		std::vector<DecodedCommand> m_queue;
	};
}

