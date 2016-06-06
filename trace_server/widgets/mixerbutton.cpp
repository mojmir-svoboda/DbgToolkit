#include "mixerbutton.h"

void MixerButton::onClicked (bool checked)
{
	m_mixer->setValueTo(m_row, m_col, checked);
}


