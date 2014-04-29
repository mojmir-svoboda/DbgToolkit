#pragma once
#include <QWidget>
#include <QColorDialog>
#include <QTableView>
#include <QAbstractProxyModel>
#include <QAbstractItemView>
#include "tableconfig.h"
#include "tablectxmenu.h"
#include "tablemodelview.h"
#include "action.h"
#include "dock.h"
#include <cmd.h>

class Connection;

namespace table {

	class TableWidget : public QTableView, public DockedWidgetBase
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

		void requestTableWheelEventSync (QWheelEvent * ev, QTableView const * source);
		void requestTableActionSync (unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QTableView const * source);

		void loadConfig (QString const & path);
		void saveConfig (QString const & path);
		void applyConfig ();
        void exportStorageToCSV (QString const & filename) { }

	protected:
		virtual void wheelEvent (QWheelEvent * event);
		virtual QModelIndex	moveCursor (CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

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

	protected:
		TableConfig m_config;
		table::CtxTableConfig m_config_ui;
		//QList<QColor> m_colors;
		QString m_fname;
		TableModel * m_modelView;
		QAbstractProxyModel * m_table_view_proxy;
		Connection * m_connection;
	};
}

