#pragma once
#include <3rd/qwt/qwt_scale_widget.h>

class ScaleWidget : public QwtScaleWidget
{
	
public:
	explicit ScaleWidget (QwtScaleDraw::Alignment alignment, QWidget * parent = NULL)
		: QwtScaleWidget(alignment, parent)
	{ }

	int scaleMargin () const
	{
		int y1,y2;
		scaleDraw()->getBorderDistHint(font(), y1, y2);
		int y = qMax(y1, y2);
		return y;
	}
};


