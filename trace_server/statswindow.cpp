#include "statswindow.h"
#include <QtGui>
#include <QMainWindow>

namespace stats {

StatsWindow::StatsWindow (QObject * parent)
	: QObject(parent)
	, m_window(0)
	, m_plot(0)
{
	qDebug("%s", __FUNCTION__);
	m_window = new QMainWindow;
	m_plot = new StatsPlot;
}
	
StatsWindow::~StatsWindow ()
{ }

}

