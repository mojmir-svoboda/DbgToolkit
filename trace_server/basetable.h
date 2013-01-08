#pragma once
#include <QtGui/qwidget.h>
#include <QColorDialog>
#include <QTableView>
#include <QAbstractProxyModel>
#include <QAbstractItemView>
#include "config.h"
#include "tableconfig.h"
#include "tablectxmenu.h"
#include "tablemodelview.h"

class Connection;

namespace table {

	class BaseTable : public QTableView
	{
		Q_OBJECT
	public:
		BaseTable (Connection * oparent, QWidget * wparent, TableConfig & cfg, QString const & fname);

		void applyConfig (TableConfig & pcfg);
		virtual ~BaseTable ();
		void stopUpdate ();

		TableConfig & getConfig () { return m_config; }
		TableConfig const & getConfig () const { return m_config; }

		void appendTableXY (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & msg);
		void appendTableSetup (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & hhdr, QString const & tag);

		bool isModelProxy () const;
		void onInvalidateFilter ();
		void findNearestTimeRow (unsigned long long t);

		void requestTableWheelEventSync (QWheelEvent * ev, QTableView const * source);
		void requestTableActionSync (unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QTableView const * source);

	protected:
		void timerEvent (QTimerEvent * e);
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
		int m_timer;
		TableConfig & m_config;
		table::CtxTableConfig m_config_ui;
		//QList<QColor> m_colors;
		QString m_fname;
		TableModelView * m_modelView;
		QAbstractProxyModel * m_table_view_proxy;
		Connection * m_connection;
	};
}

