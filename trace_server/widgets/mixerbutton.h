#pragma once
#include <QToolButton>
#include <QMouseEvent>
#include "mixer.h"
#include "rubberband.h"

struct MixerButton : QToolButton
{
	Mixer * m_mixer;
	int m_row;
	int m_col;

  MixerButton (Mixer * mixer, int row, int col, QWidget *parent = nullptr)
		: QToolButton(parent)
		, m_row(row)
		, m_col(col)
		, m_mixer(mixer)
	{
		setMinimumSize(QSize(16, 16));
		setMaximumSize(QSize(16, 16));		
		setCheckable(true);
		setChecked(false);

	//QString ss = "QToolButton:pressed{background-color: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 : 1,stop : 0 #dadbde, stop: 1 #f6f7fa);}";
		//setStyleSheet(ss);
	}

	bool wantRubberBand () const
	{
		return QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier)
			|| QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier);
	}

	QPoint eventPosToMixer (QPoint const & loc_pt)
	{
		QPoint const glob_pt = mapToGlobal(loc_pt);
		QPoint const mix_pt = m_mixer->mapFromGlobal(glob_pt);
		return mix_pt;
	}

	void mousePressEvent (QMouseEvent * event)
	{
		if (wantRubberBand())
		{
			m_mixer->startRubberBand(eventPosToMixer(event->pos()));
		}
		else
			QToolButton::mousePressEvent(event);
	}
	void mouseMoveEvent (QMouseEvent * event)
	{
		if (wantRubberBand())
			m_mixer->enlargeRubberBand(eventPosToMixer(event->pos()));
		else
			QToolButton::mouseMoveEvent(event);
	}
	void mouseReleaseEvent (QMouseEvent * event)
	{
		if (wantRubberBand())
			m_mixer->stopRubberBand(eventPosToMixer(event->pos()));
		else
			QToolButton::mouseReleaseEvent(event);
	}

Q_OBJECT
public slots:
	void onClicked (bool checked);
};
