#include "ganttview.h"
#include <QtGui>
#include <QSpinBox>
#ifndef QT_NO_OPENGL
#	include <QtOpenGL>
#endif
#include <qmath.h>

namespace gantt {

int g_heightValue = 38;
int g_spaceValue = 15;
float g_scaleValue = 1.0f;

void GanttView::appendGanttBgn (QString const & time, QString const & tid, QString const & fgc, QString const & bgc, QString const & tag)
{
}

GanttView::GanttView (Connection * conn, QWidget * parent, gantt::GanttViewConfig & config, QString const & fname)
	: QFrame(parent)
	, m_connection(conn)
{
	setFrameStyle(Sunken | StyledPanel);
	m_frameSpinBox = new QSpinBox;
	m_graphicsView = new GraphicsView();
	m_graphicsView->setRenderHint(QPainter::Antialiasing, false);
	m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
	m_graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
	m_graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

	int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
	QSize iconSize(size, size);

/*	QToolButton * zoomInIcon = new QToolButton;
	zoomInIcon->setAutoRepeat(true);
	zoomInIcon->setAutoRepeatInterval(33);
	zoomInIcon->setAutoRepeatDelay(0);
	zoomInIcon->setIcon(QPixmap(":/zoomin.png"));
	zoomInIcon->setIconSize(iconSize);
	QToolButton * zoomOutIcon = new QToolButton;
	zoomOutIcon->setAutoRepeat(true);
	zoomOutIcon->setAutoRepeatInterval(33);
	zoomOutIcon->setAutoRepeatDelay(0);
	zoomOutIcon->setIcon(QPixmap(":/zoomout.png"));
	zoomOutIcon->setIconSize(iconSize);*/
	m_zoomSlider = new QSlider;
	m_zoomSlider->setMinimum(0);
	m_zoomSlider->setMaximum(500);
	m_zoomSlider->setValue(250);
	m_zoomSlider->setTickPosition(QSlider::TicksRight);

	// Zoom slider layout
	QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
	//zoomSliderLayout->addWidget(zoomInIcon);
	zoomSliderLayout->addWidget(m_zoomSlider);
	//zoomSliderLayout->addWidget(zoomOutIcon);
/*
	m_resetButton = new QToolButton;
	m_resetButton->setText(tr("0"));
	m_resetButton->setEnabled(false);

	// Label layout
	QHBoxLayout *labelLayout = new QHBoxLayout;
	m_label = new QLabel(name);
	m_antialiasButton = new QToolButton;
	m_antialiasButton->setText(tr("Antialiasing"));
	m_antialiasButton->setCheckable(true);
	m_antialiasButton->setChecked(false);
	m_openGlButton = new QToolButton;
	m_openGlButton->setText(tr("OpenGL"));
	m_openGlButton->setCheckable(true);
#ifndef QT_NO_OPENGL
	m_openGlButton->setEnabled(QGLFormat::hasOpenGL());
#else
	m_openGlButton->setEnabled(false);
#endif

	labelLayout->addWidget(m_label);
	labelLayout->addStretch();
	labelLayout->addWidget(m_antialiasButton);
	labelLayout->addWidget(m_openGlButton);
*/
	QGridLayout *topLayout = new QGridLayout;
	//topLayout->addLayout(labelLayout, 0, 0);
	topLayout->addWidget(m_graphicsView, 1, 0);
	topLayout->addLayout(zoomSliderLayout, 1, 1);

	//QSlider * m_heightSlider = new QSlider;
	//m_heightSlider->setMinimum(0);
	//m_heightSlider->setMaximum(400);
	//m_heightSlider->setValue(g_heightValue);
	//topLayout->addWidget(m_heightSlider, 1, 2);
	//m_heightSlider->setTickPosition(QSlider::TicksRight);
	//
	m_frameSpinBox->setMinimum(0);
	m_frameSpinBox->setValue(0);

	//int const max = m_mainWindow->getProfileInfo(0).m_frames.size();
	//m_frameSpinBox->setMaximum(max);
	//topLayout->addWidget(m_frameSpinBox, 0, 1);

	//topLayout->addWidget(m_resetButton, 2, 1);
	setLayout(topLayout);

	//connect(m_resetButton, SIGNAL(clicked()), this, SLOT(resetView()));
	//connect(m_zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
	connect(m_graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(setResetButtonEnabled()));
	connect(m_graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(setResetButtonEnabled()));
	//connect(m_antialiasButton, SIGNAL(toggled(bool)), this, SLOT(toggleAntialiasing()));
	//connect(m_openGlButton, SIGNAL(toggled(bool)), this, SLOT(toggleOpenGL()));
	//connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
	//connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));

	//connect(m_heightSlider, SIGNAL(valueChanged(int)), this, SLOT(changeHeight(int)));

	setupMatrix();
}

QGraphicsView * GanttView::view() const { return m_graphicsView; }

void GanttView::resetView()
{
	//m_zoomSlider->setValue(250);
	setupMatrix();
	m_graphicsView->ensureVisible(QRectF(0, 0, 0, 0));

	//m_resetButton->setEnabled(false);
}

void GanttView::changeHeight (int n)
{
	g_heightValue = n;
	QGraphicsScene * scene = view()->scene();
	view()->setScene(0);

	scene->clear();
	//m_mainWindow->populateScene();
	//view()->setScene(scene);
	//m_graphicsView->viewport()->update();
}

void GanttView::setResetButtonEnabled()
{
	//m_resetButton->setEnabled(true);
}

void GanttView::setupMatrix()
{
	qreal scale = qPow(qreal(2), (m_zoomSlider->value() - 250) / qreal(50));

	QMatrix matrix;
	matrix.scale(scale, scale);

	m_graphicsView->setMatrix(matrix);
	setResetButtonEnabled();
}

void GanttView::toggleOpenGL()
{
#ifndef QT_NO_OPENGL
	//m_graphicsView->setViewport(m_openGlButton->isChecked() ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
#endif
}

void GanttView::toggleAntialiasing()
{
	//m_graphicsView->setRenderHint(QPainter::Antialiasing, m_antialiasButton->isChecked());
}

void GanttView::zoomIn()
{
	m_zoomSlider->setValue(m_zoomSlider->value() + 1);
}

void GanttView::zoomOut()
{
	m_zoomSlider->setValue(m_zoomSlider->value() - 1);
}

void GanttView::forceUpdate ()
{
	m_graphicsView->viewport()->update();
}

GraphicsView::GraphicsView (QWidget * parent)
	: QGraphicsView(parent)
{ }

/**
  * Sets the current centerpoint.  Also updates the scene's center point.
  * Unlike centerOn, which has no way of getting the floating point center
  * back, SetCenter() stores the center point.	It also handles the special
  * sidebar case.  This function will claim the centerPoint to sceneRec ie.
  * the centerPoint must be within the sceneRec.
  */
void GraphicsView::SetCenter (QPointF const & centerPoint)
{
	// Get the rectangle of the visible area in scene coords
	QRectF visibleArea = mapToScene(rect()).boundingRect();
	// Get the scene area
	QRectF sceneBounds = sceneRect();

	double const boundX = visibleArea.width() / 2.0;
	double const boundY = visibleArea.height() / 2.0;
	double const boundWidth = sceneBounds.width() - 2.0 * boundX;
	double const boundHeight = sceneBounds.height() - 2.0 * boundY;

	//qDebug("setcenter: x=%f y=%f w=%f h=%f", boundX, boundY, boundWidth, boundHeight);
	// The max boundary that the centerPoint can be to
	QRectF bounds(boundX, boundY, boundWidth, boundHeight);
	if (bounds.contains(centerPoint))
	{
		// We are within the bounds
		CurrentCenterPoint = centerPoint;
	}
	else
	{
		// We need to clamp or use the center of the screen
		if (visibleArea.contains(sceneBounds))
		{
			// Use the center of scene ie. we can see the whole scene
			CurrentCenterPoint = centerPoint;
			//CurrentCenterPoint = sceneBounds.center();
		}
		else
		{
			CurrentCenterPoint = centerPoint;
 
			//We need to clamp the center. The centerPoint is too large
			/*if (centerPoint.x() > bounds.x() + bounds.width()) {
				CurrentCenterPoint.setX(bounds.x() + bounds.width());
			} else if (centerPoint.x() < bounds.x()) {
				CurrentCenterPoint.setX(bounds.x());
			}
 
			if (centerPoint.y() > bounds.y() + bounds.height()) {
				CurrentCenterPoint.setY(bounds.y() + bounds.height());
			} else if (centerPoint.y() < bounds.y()) {
				CurrentCenterPoint.setY(bounds.y());
			}*/
		}
	}
	// Update the scrollbars
	centerOn(CurrentCenterPoint);
}
 
void GraphicsView::mousePressEvent (QMouseEvent * event)
{
	// For panning the view
	LastPanPoint = event->pos();
	setCursor(Qt::ClosedHandCursor);
}
 
void GraphicsView::mouseReleaseEvent (QMouseEvent * event)
{
	setCursor(Qt::OpenHandCursor);
	LastPanPoint = QPoint();
}
 
void GraphicsView::mouseMoveEvent (QMouseEvent * event)
{
	if (!LastPanPoint.isNull())
	{
		//Get how much we panned
		QPointF delta = mapToScene(LastPanPoint) - mapToScene(event->pos());
		LastPanPoint = event->pos();

		QPointF cen = mapToScene(viewport()->rect()).boundingRect().center();
		SetCenter(cen + delta);
 
		//Update the center ie. do the pan
		//SetCenter(GetCenter() + delta);
		//qDebug("new center: %f %f", GetCenter().x(), GetCenter().y()); 
	}
	else
	{
		QGraphicsView::mouseMoveEvent(event);
	}
}
 
/**
  * Zoom the view in and out.
  */
void GraphicsView::wheelEvent (QWheelEvent* event)
{
	bool const shift = event->modifiers() & Qt::SHIFT;

	if (shift)
	{
		//m_frameSpinBox.setValue(m_frameSpinBox.value() + event->delta());
	}
	else
	{
		//Get the position of the mouse before scaling, in scene coords
		QPointF pointBeforeScale(mapToScene(event->pos()));

		//Get the original screen centerpoint
		QPointF screenCenter = GetCenter(); //CurrentCenterPoint; //(visRect.center());

		//Scale the view ie. do the zoom
		double scaleFactor = 1.15; //How fast we zoom
		if (event->delta() > 0) {
			//Zoom in
			scale(scaleFactor, scaleFactor);
		} else {
			//Zooming out
			scale(1.0 / scaleFactor, 1.0 / scaleFactor);
		}
	 
		//Get the position after scaling, in scene coords
		QPointF pointAfterScale(mapToScene(event->pos()));
	 
		//Get the offset of how the screen moved
		QPointF offset = pointBeforeScale - pointAfterScale;
	 
		//Adjust to the new center for correct zooming
		QPointF newCenter = screenCenter + offset;
		SetCenter(newCenter);
	}
}
 
/**
  * Need to update the center so there is no jolt in the
  * interaction after resizing the widget.
  */
void GraphicsView::resizeEvent (QResizeEvent * event)
{
	//Get the rectangle of the visible area in scene coords
	QRectF visibleArea = mapToScene(rect()).boundingRect();
	SetCenter(visibleArea.center());
 
	//Call the subclass resize so the scrollbars are updated correctly
	QGraphicsView::resizeEvent(event);
}

}

