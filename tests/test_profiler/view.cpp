#include "view.h"
#include <QtGui>
#include <QSpinBox>
#ifndef QT_NO_OPENGL
#	include <QtOpenGL>
#endif
#include <qmath.h>
#include "mygraphicsview.h"
#include "mainwindow.h"

int g_heightValue = 38;
int g_spaceValue = 15;
float g_scaleValue = 1.0f;

View::View (MainWindow * mw, const QString & name, QWidget * parent)
	: m_mainWindow(mw)
	, QFrame(parent)
{
	setFrameStyle(Sunken | StyledPanel);
	m_frameSpinBox = new QSpinBox;
	m_graphicsView = new MyGraphicsView(*m_frameSpinBox);
	m_graphicsView->setRenderHint(QPainter::Antialiasing, false);
	m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
	m_graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
	m_graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

	int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
	QSize iconSize(size, size);

	QToolButton * zoomInIcon = new QToolButton;
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
	zoomOutIcon->setIconSize(iconSize);
	m_zoomSlider = new QSlider;
	m_zoomSlider->setMinimum(0);
	m_zoomSlider->setMaximum(500);
	m_zoomSlider->setValue(250);
	m_zoomSlider->setTickPosition(QSlider::TicksRight);

	// Zoom slider layout
	QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
	zoomSliderLayout->addWidget(zoomInIcon);
	zoomSliderLayout->addWidget(m_zoomSlider);
	zoomSliderLayout->addWidget(zoomOutIcon);

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

	QGridLayout *topLayout = new QGridLayout;
	topLayout->addLayout(labelLayout, 0, 0);
	topLayout->addWidget(m_graphicsView, 1, 0);
	topLayout->addLayout(zoomSliderLayout, 1, 1);

	QSlider * m_heightSlider = new QSlider;
	m_heightSlider->setMinimum(0);
	m_heightSlider->setMaximum(400);
	m_heightSlider->setValue(g_heightValue);
	topLayout->addWidget(m_heightSlider, 1, 2);
	//m_heightSlider->setTickPosition(QSlider::TicksRight);
	//
	m_frameSpinBox->setMinimum(0);
	m_frameSpinBox->setValue(0);

	int const max = m_mainWindow->getProfileInfo(0).m_frames.size();
	m_frameSpinBox->setMaximum(max);
	topLayout->addWidget(m_frameSpinBox, 0, 1);

	topLayout->addWidget(m_resetButton, 2, 1);
	setLayout(topLayout);

	connect(m_resetButton, SIGNAL(clicked()), this, SLOT(resetView()));
	connect(m_zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
	//connect(m_graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(setResetButtonEnabled()));
	//connect(m_graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(setResetButtonEnabled()));
	connect(m_antialiasButton, SIGNAL(toggled(bool)), this, SLOT(toggleAntialiasing()));
	connect(m_openGlButton, SIGNAL(toggled(bool)), this, SLOT(toggleOpenGL()));
	connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));

	connect(m_heightSlider, SIGNAL(valueChanged(int)), this, SLOT(changeHeight(int)));

	setupMatrix();
}

QGraphicsView * View::view() const { return m_graphicsView; }

void View::resetView()
{
	m_zoomSlider->setValue(250);
	setupMatrix();
	m_graphicsView->ensureVisible(QRectF(0, 0, 0, 0));

	m_resetButton->setEnabled(false);
}

void View::changeHeight (int n)
{
	g_heightValue = n;
	QGraphicsScene * scene = view()->scene();
	view()->setScene(0);

	scene->clear();
	m_mainWindow->populateScene();
	view()->setScene(scene);
	m_graphicsView->viewport()->update();
}

void View::setResetButtonEnabled()
{
	m_resetButton->setEnabled(true);
}

void View::setupMatrix()
{
	qreal scale = qPow(qreal(2), (m_zoomSlider->value() - 250) / qreal(50));

	QMatrix matrix;
	matrix.scale(scale, scale);

	m_graphicsView->setMatrix(matrix);
	setResetButtonEnabled();
}

void View::toggleOpenGL()
{
#ifndef QT_NO_OPENGL
	m_graphicsView->setViewport(m_openGlButton->isChecked() ? new QGLWidget(QGLFormat(QGL::SampleBuffers)) : new QWidget);
#endif
}

void View::toggleAntialiasing()
{
	m_graphicsView->setRenderHint(QPainter::Antialiasing, m_antialiasButton->isChecked());
}

void View::zoomIn()
{
	m_zoomSlider->setValue(m_zoomSlider->value() + 1);
}

void View::zoomOut()
{
	m_zoomSlider->setValue(m_zoomSlider->value() - 1);
}


