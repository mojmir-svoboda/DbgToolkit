#pragma once
#include <QWidget>
#include "config.h"
#include "ganttconfig.h"
#include "ganttctxmenu.h"

#include <QFrame>
#include <QGraphicsView>
#include <cstdio>

class Connection;

namespace gantt {
	QT_FORWARD_DECLARE_CLASS(GraphicsView)
}
//QT_FORWARD_DECLARE_CLASS(QGraphicsView)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QSpinBox)

namespace gantt {


	class GraphicsView : public QGraphicsView
	{
		Q_OBJECT;
	public:
		GraphicsView (QSpinBox & fs, QWidget* parent = NULL);

		//Set the current centerpoint in the
		void SetCenter (QPointF const & centerPoint);
		void ForceCenter (QPointF const & center) { CurrentCenterPoint = center; }
	 
	protected:
		//Holds the current centerpoint for the view, used for panning and zooming
		QPointF CurrentCenterPoint;
		QPointF GetCenter () { return CurrentCenterPoint; }
	 
		QPoint LastPanPoint; //From panning the view
	 
		virtual void mousePressEvent (QMouseEvent* event);
		virtual void mouseReleaseEvent (QMouseEvent* event);
		virtual void mouseMoveEvent (QMouseEvent* event);
		virtual void wheelEvent (QWheelEvent* event);
		virtual void resizeEvent (QResizeEvent* event);

		QSpinBox & m_frameSpinBox;
	};



	class View : public QFrame
	{
		Q_OBJECT
	public:
		View (Connection * oparent, QWidget * parent, QString const & fname);

		QGraphicsView * view () const;
		void forceUpdate ();

	private slots:
		void resetView ();
		void setResetButtonEnabled ();
		void setupMatrix ();
		void toggleOpenGL ();
		void toggleAntialiasing ();

		void zoomIn ();
		void zoomOut ();
		void changeHeight (int n);

		
	private:
		GraphicsView * m_graphicsView;
		//QLabel * m_label;
		//QToolButton * m_openGlButton;
		//QToolButton * m_antialiasButton;
		//QToolButton * m_resetButton;
		QSlider * m_zoomSlider;
		QSpinBox * m_frameSpinBox;
		Connection * m_connection;
	};


	class BaseGantt : public QFrame
	{
		Q_OBJECT
	public:
		BaseGantt (Connection * oparent, QWidget * wparent, GanttConfig & cfg, QString const & fname);

		void applyConfig (GanttConfig & pcfg);
		virtual ~BaseGantt ();

		GanttConfig & getConfig () { return m_config; }
		GanttConfig const & getConfig () const { return m_config; }

		void appendGantt (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & msg);

		//void findNearestTimeRow (unsigned long long t);
		//void requestWheelEventSync (QWheelEvent * ev, QGanttView const * source);
		//void requestActionSync (unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QGanttView const * source);

	protected:
		//virtual void wheelEvent (QWheelEvent * event);
		//virtual QModelIndex	moveCursor (CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

	public Q_SLOTS:

		void onShow ();
		void onHide ();
		void onHideContextMenu ();
		void onShowContextMenu (QPoint const & pos);
		void setConfigValuesToUI (GanttConfig const & cfg);
		void setUIValuesToConfig (GanttConfig & cfg);
		void onSaveButton ();
		void onApplyButton ();
		void onResetButton ();
		void onDefaultButton ();
		//void scrollTo (QModelIndex const & index, ScrollHint hint);

	protected:
		GanttConfig & m_config;
		gantt::CtxGanttConfig m_config_ui;
		QString m_fname;
		Connection * m_connection;
	};
}

