#pragma once

inline void setCheckedWithBlockedSignals (QCheckBox * w, bool val)
{
  bool const old = w->blockSignals(true);
  w->setChecked(val);
  w->blockSignals(old);
}

template<typename T>
inline void setValueWithBlockedSignals (QSpinBox * w, T const & val)
{
  bool const old = w->blockSignals(true);
  w->setValue(val);
  w->blockSignals(old);
}

template<class C, typename T>
inline void setCurrentIndexWithBlockedSignals (C * w, T const & index)
{
  bool const old = w->blockSignals(true);
  w->setCurrentIndex(index);
  w->blockSignals(old);
}




