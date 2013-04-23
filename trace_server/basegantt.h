#pragma once
#include <QWidget>
#include "config.h"
#include "ganttconfig.h"
#include "ganttctxmenu.h"
#include <QFrame>
#include <cstdio>

class Connection;

namespace gantt {
	QT_FORWARD_DECLARE_CLASS(GanttView)
}

namespace gantt {

	class BaseGantt : public QFrame
	{
		Q_OBJECT
	public:
		BaseGantt (Connection * oparent, QWidget * wparent, GanttConfig & cfg, QString const & fname);

		void applyConfig (GanttConfig & pcfg);
		virtual ~BaseGantt ();

		GanttConfig & getConfig () { return m_config; }
		GanttConfig const & getConfig () const { return m_config; }

		typedef QMap<QString, GanttView *> ganttviews_t;
		GanttView * findGanttView (QString const & subtag);
		GanttView * findOrCreateGanttView (QString const & subtag);
		ganttviews_t::iterator mkGanttView (QString const & subtag);

		//void appendGantt (QString const & time, QString const & tid, QString const & fgc, QString const & bgc, QString const & msg);

		//void findNearestTimeRow (unsigned long long t);
		//void requestWheelEventSync (QWheelEvent * ev, QGanttView const * source);
		//void requestActionSync (unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QGanttView const * source);

	protected:
		//virtual void wheelEvent (QWheelEvent * event);
		//virtual QModelIndex	moveCursor (CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

	public Q_SLOTS:

		void showGanttView (GanttView * item, bool on);

		void onShow ();
		void onHide ();
		void onHideContextMenu ();
		void onShowContextMenu (QPoint const & pos);
		void setConfigValuesToUI (GanttConfig const & cfg);
		void setUIValuesToConfig (GanttConfig & cfg);
		void setViewConfigValuesToUI (GanttViewConfig const & gvcfg);
		void setUIValuesToViewConfig (GanttViewConfig & cfg);
		void onSaveButton ();
		void onApplyButton ();
		void onResetButton ();
		void onDefaultButton ();
		//void scrollTo (QModelIndex const & index, ScrollHint hint);
		
		void onGanttViewActivate (int idx);
		void onClearAllDataButton ();
		void onClearGanttViewDataButton ();


	protected:
		GanttConfig & m_config;
		gantt::CtxGanttConfig m_config_ui;
		QString m_fname;
		Connection * m_connection;
		QGridLayout * m_layout;
		ganttviews_t m_ganttviews;
		QVector<QString> m_subtags;
	};
}

