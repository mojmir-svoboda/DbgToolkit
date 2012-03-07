#pragma once
#include <QFrame>
#include <QGraphicsView>
#include <cstdio>

QT_FORWARD_DECLARE_CLASS(MyGraphicsView)
QT_FORWARD_DECLARE_CLASS(QGraphicsView)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QToolButton)

class View : public QFrame
{
    Q_OBJECT
public:
    View (const QString & name, QWidget * parent = 0);

    QGraphicsView * view () const;

private slots:
    void resetView ();
    void setResetButtonEnabled ();
    void setupMatrix ();
    void toggleOpenGL ();
    void toggleAntialiasing ();

    void zoomIn ();
    void zoomOut ();
    
private:
    MyGraphicsView * m_graphicsView;
    QLabel * m_label;
    QToolButton * m_openGlButton;
    QToolButton * m_antialiasButton;
    QToolButton * m_resetButton;
    QSlider * m_zoomSlider;
};

