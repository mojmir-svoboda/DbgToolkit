#pragma once
#include <QWidget>
#include <QDockWidget>
#include <QFrame>
#include "ganttconfig.h"
#include "ganttdata.h"
#include "ganttctxmenu.h"
#include "frameview.h"

class Connection;
QT_FORWARD_DECLARE_CLASS(QSplitter)

namespace gantt {
	QT_FORWARD_DECLARE_CLASS(GanttView)
}

struct DataFrameView {
	Connection * m_parent;
	QDockWidget * m_wd;
	FrameView * m_widget;
	FrameViewConfig m_config;
	QString m_fname;

	DataFrameView (Connection * parent, FrameViewConfig & config, QString const & fname);
	~DataFrameView ();

	void onShow ();
	void onHide ();
	FrameView & widget () { return *m_widget; }
};

typedef QMap<int, DataFrameView *> dataframeviews_t;



namespace gantt {

	class GanttWidget : public QFrame
	{
		Q_OBJECT
	public:
		GanttWidget (Connection * oparent, QWidget * wparent, GanttConfig & cfg, QString const & fname);

		void applyConfig (GanttConfig & pcfg);
		virtual ~GanttWidget ();

		GanttConfig & getConfig () { return m_config; }
		GanttConfig const & getConfig () const { return m_config; }

		typedef QMap<QString, GanttView *> ganttviews_t;
		GanttView * findGanttView (QString const & subtag);
		GanttView * findOrCreateGanttView (QString const & subtag);
		ganttviews_t::iterator mkGanttView (QString const & subtag);

		dataframeviews_t::iterator findOrCreateFrameView (int sync_group);


		//void appendGantt (QString const & time, QString const & tid, QString const & fgc, QString const & bgc, QString const & msg);
		void appendFrameEnd (DecodedData & data);

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
		void onFitAllButton ();
		void onFitFrameButton ();
		void onPrevFrameButton ();
		void onNextFrameButton ();
		void onFrameValueChanged (int);
		//void scrollTo (QModelIndex const & index, ScrollHint hint);
		
		void onGanttViewActivate (int idx);
		void onClearAllDataButton ();
		void onClearGanttViewDataButton ();

		void performTimeSynchronization (int sync_group, unsigned long long time, void * source);
		void performFrameSynchronization (int sync_group, unsigned long long frame, void * source);

	signals:
		void requestTimeSynchronization (int sync_group, unsigned long long time, void * source);
		void requestFrameSynchronization (int sync_group, unsigned long long frame, void * source);

	protected:
		GanttConfig & m_config;
		gantt::CtxGanttConfig m_config_ui;
		QString m_fname;
		Connection * m_connection;
		QSplitter * m_layout;
		ganttviews_t m_ganttviews;
		QVector<QString> m_subtags;
		dataframeviews_t m_dataframeviews;
	};
}

