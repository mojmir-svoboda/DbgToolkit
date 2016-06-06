#include "mixerbar.h"
#include "ui_mixerbar.h"
#include "mixer.h"

MixerBar::MixerBar (Mixer * parent, int row)
	: QWidget(parent)
	, m_ui(new Ui::MixerBar)
	, m_mixer(parent)
	, m_row(row)
{
    m_ui->setupUi(this);
}

MixerBar::~MixerBar()
{
    delete m_ui;
}

void MixerBar::onClickedOnOn ()
{
	m_mixer->setRowTo(m_row, true);
}

void MixerBar::onClickedOnOff ()
{
	m_mixer->setRowTo(m_row, false);
}

