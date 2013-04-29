#pragma once
#include <QFrame>
#include "qwt/qwt_plot_barchart.h"
#include "frameviewconfig.h"

class Connection;

struct FrameView : QFrame, QwtPlotBarChart
{
	FrameView (Connection * oparent, QWidget * wparent, FrameViewConfig & cfg, QString const & fname);

	void appendFrame (unsigned long long from, unsigned long long to);

	virtual QwtText barTitle (int idx) const
	{
		QwtText title;
		if (idx >= 0 && idx < m_strvalues.size())
			title = m_strvalues[idx];
		return title;
	}

	void onShow ();
	void onHide ();
	//void onHideContextMenu ();
	//void onShowContextMenu (QPoint const & pos);

public:

	QVector<double> m_values;
	std::vector<QString> m_strvalues;
	std::vector<QColor> m_colors;
};

