#pragma once
#include "blockinfo.h"
#include <QtGui/qwidget.h>

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)
QT_FORWARD_DECLARE_CLASS(QGraphicsView)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QSplitter)

class MainWindow : public QWidget
{
	Q_OBJECT
public:
	MainWindow (std::vector<ProfileInfo> & pis, QWidget * parent = 0);
	void populateScene ();

	ProfileInfo const & getProfileInfo (size_t idx = 0) const { return m_profileInfos[idx]; }
	
private:
	void setupMatrix ();
	
	QGraphicsScene * m_scene;
	std::vector<ProfileInfo> & m_profileInfos;
};
