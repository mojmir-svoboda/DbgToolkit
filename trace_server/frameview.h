#pragma once
#include "histogram.h"
#include "frameviewconfig.h"

class Connection;

struct FrameView : Histogram
{
	FrameView (Connection * oparent, QWidget * wparent, FrameViewConfig & cfg, QString const & fname);

	void appendFrame ();
};

