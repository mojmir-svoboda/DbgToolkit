#pragma once

inline QtColorPicker * mkColorPicker (QWidget * parent, QString const & txt, QColor const & c)
{
  QtColorPicker * w = new QtColorPicker(parent, txt);
  w->setStandardColors();
  w->setCurrentColor(c);
  return w;
}


