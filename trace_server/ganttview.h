#pragma once
#include <QWidget>
#include "config.h"
#include "ganttconfig.h"
//#include "ganttctxmenu.h"

#include <QFrame>
#include <QGraphicsView>
//#include <cstdio>

class Connection;

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
		GraphicsView (QWidget* parent = NULL);

		//Set the current centerpoint in the view
		void SetCenter (QPointF const & centerPoint);
		void ForceCenter (QPointF const & center) { CurrentCenterPoint = center; }
	 
	protected:
		
		QPointF CurrentCenterPoint; ///Holds the current centerpoint for the view, used for panning and zooming
		QPointF GetCenter () { return CurrentCenterPoint; }
	 
		QPoint LastPanPoint;		///From panning the view
	 
		virtual void mousePressEvent (QMouseEvent* event);
		virtual void mouseReleaseEvent (QMouseEvent* event);
		virtual void mouseMoveEvent (QMouseEvent* event);
		virtual void wheelEvent (QWheelEvent* event);
		virtual void resizeEvent (QResizeEvent* event);
	};



	class GanttView : public QFrame
	{
		Q_OBJECT
	public:

		GanttView (Connection * oparent, QWidget * parent, GanttViewConfig & gvcfg, QString const & fname);

		QGraphicsView * view () const;
		void forceUpdate ();

		void appendGanttBgn (QString const & time, QString const & tid, QString const & fgc, QString const & bgc, QString const & tag);

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
		QGraphicsScene * m_graphicsScene;
		//QLabel * m_label;
		//QToolButton * m_openGlButton;
		//QToolButton * m_antialiasButton;
		//QToolButton * m_resetButton;
		QSlider * m_zoomSlider;
		QSpinBox * m_frameSpinBox;
		Connection * m_connection;
	};
}

