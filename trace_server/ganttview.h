#pragma once
#include <QWidget>
#include <QMap>
#include "config.h"
#include "ganttconfig.h"
#include "ganttdata.h"
#include "scalewidget.h"
#include "qwt/qwt_transform.h"

#include <QFrame>
#include <QGraphicsView>
#include <QGridLayout>
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


	struct GfxView {
		QGraphicsScene * m_scene;
		GraphicsView *   m_view;

		GfxView () { memset(this, 0, sizeof(*this)); }
	};

	class GanttView : public QFrame
	{
		Q_OBJECT
	public:

		GanttView (Connection * oparent, QWidget * parent, GanttViewConfig & gvcfg, QString const & fname);

		//QGraphicsView * view () const;
		void forceUpdate ();

		void appendGantt (DecodedData & data);

		void updateTimeWidget (GraphicsView * v);

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

		void initColors ();
		void appendBgn (DecodedData & data);
		void appendEnd (DecodedData & data);
		void appendFrameBgn (DecodedData & data);
		void appendFrameEnd (DecodedData & data);
		void consumeData (contextdata_t *);

		typedef QMap<unsigned long long, GfxView> contextviews_t;
		contextviews_t m_contextviews;
		GfxView & createViewForContext (unsigned long long ctx, QGraphicsScene * scene = 0);

		GfxView & viewAt (unsigned long long ctx);

		QGridLayout * m_layout;
		Connection * m_connection;
		GanttViewConfig & m_gvcfg;
		GanttData m_ganttData;
		unsigned m_last_flush_end_idx;

		static size_t const m_max_unique_colors = 256;
		std::vector<QColor> m_unique_colors;
		std::vector<QString> m_tags;

		typedef QMap<QString, QColor> colormap_t;
		colormap_t m_tagcolors;
		std::map<unsigned, unsigned> m_max_layers;
		ScaleWidget * m_timewidget;
	};
}

