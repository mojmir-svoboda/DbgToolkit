#pragma once
#include <QFrame>
#include <QGraphicsView>
#include <cstdio>

namespace profiler {
	QT_FORWARD_DECLARE_CLASS(GraphicsView)
}
QT_FORWARD_DECLARE_CLASS(QGraphicsView)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QToolButton)
QT_FORWARD_DECLARE_CLASS(QSpinBox)

namespace profiler {

	extern int g_heightValue;
	extern int g_spaceValue;
	extern float g_scaleValue;

	class ProfilerWindow;

	class View : public QFrame
	{
		Q_OBJECT
	public:
		View (ProfilerWindow * mw, const QString & name, QWidget * parent = 0);

		QGraphicsView * view () const;

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
		ProfilerWindow * m_mainWindow;
		GraphicsView * m_graphicsView;
		QLabel * m_label;
		QToolButton * m_openGlButton;
		QToolButton * m_antialiasButton;
		QToolButton * m_resetButton;
		QSlider * m_zoomSlider;
		QSpinBox * m_frameSpinBox;
	};
}

