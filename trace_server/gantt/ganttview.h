#pragma once
#include <QWidget>
#include <QMap>
#include "ganttconfig.h"
#include "ganttdata.h"
#include "scalewidget.h"

#include <QFrame>
#include <QGraphicsView>

class Connection;

//QT_FORWARD_DECLARE_CLASS(QGraphicsView)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QSpinBox)
QT_FORWARD_DECLARE_CLASS(QSplitter)

namespace gantt {

	class GanttView;

	class GraphicsView : public QGraphicsView
	{
		Q_OBJECT;
	public:
		GraphicsView (GanttView & gv, GanttViewConfig & gvcfg, QWidget* parent = NULL);

		//Set the current centerpoint in the view
		void setCenter (QPointF const & centerPoint);
		void forceCenter (QPointF const & center) { m_current_center = center; }
		void fitInView (QRectF const & rect, Qt::AspectRatioMode aspectRatioMode);
	 
	protected:
		
		QPointF m_current_center; ///Holds the current centerpoint for the view, used for panning and zooming
		QPointF getCenter () { return m_current_center; }
	 
		QPoint LastPanPoint;		///From panning the view
	 
		virtual void mousePressEvent (QMouseEvent * event);
		virtual void mouseReleaseEvent (QMouseEvent * event);
		virtual void mouseMoveEvent (QMouseEvent * event);
		virtual void wheelEvent (QWheelEvent * event);
		virtual void resizeEvent (QResizeEvent * event);

		GanttView & m_gv;
		GanttViewConfig & m_gvcfg;

	private slots:
		void verticalScroll (int n);
		void horizontalScroll (int n);

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
		void applyConfig (GanttViewConfig & gvcfg);

		void gotoFrame (unsigned n);
		GanttViewConfig const & config () const { return m_gvcfg; }

		void syncGanttView (GanttView const * src, QPointF const & center);
		void syncCtxViews (GraphicsView const * src, QPointF const & center);

	private slots:
		void resetView ();
		//void verticalScroll (int);
		//void horizontalScroll (int);
		void setupMatrix ();
		void toggleOpenGL ();
		void toggleAntialiasing ();

		void zoomIn ();
		void zoomOut ();
		
	private:
		friend class GanttWidget;
		friend class GraphicsView;

		void initColors ();
		void appendBgn (DecodedData & data);
		void appendEnd (DecodedData & data);
		void appendFrameBgn (DecodedData & data);
		void appendFrameEnd (DecodedData & data);
		void appendFrameEnd (DecodedData & data, unsigned long long & from, unsigned long long & to);
		void consumeData (contextdata_t *);
		void consumeEnd (Data * end_data);

		typedef QMap<unsigned long long, GfxView> contextviews_t;
		contextviews_t m_contextviews;
		GfxView & createViewForContext (unsigned long long ctx, QGraphicsScene * scene = 0);

		GfxView & viewAt (unsigned long long ctx);
	
		GanttWidget * m_ganttwidget;
		QSplitter * m_layout;
		ScaleWidget * m_timewidget;
		Connection * m_connection;
		GanttViewConfig & m_gvcfg;
		QString m_curr_strtime_units;
		double m_curr_timeunits;
		GanttData m_ganttData;
		unsigned m_last_flush_end_idx;

		static size_t const m_max_unique_colors = 256;
		std::vector<QColor> m_unique_colors;
		std::vector<QString> m_tags;

		typedef QMap<QString, QColor> colormap_t;
		colormap_t m_tagcolors;
		std::map<unsigned, unsigned> m_max_layers;

	};
}

