#include "mixerbarv.h"
#include "mixer.h"
#include "ui_mixerbarv.h"

MixerBarV::MixerBarV (QWidget * parent, Mixer * mixer, int col)
	: QWidget(parent)
	, m_ui(new Ui::MixerBarV)
	, m_mixer(mixer)
	, m_col(col)
{
   m_ui->setupUi(this);
}

MixerBarV::~MixerBarV ()
{
   delete m_ui;
}

void MixerBarV::onClickedOnOn ()
{
	m_mixer->setColTo(m_col, true);
}

void MixerBarV::onClickedOnOff ()
{
	m_mixer->setColTo(m_col, false);
}
