#pragma once
#include <QWidget>

namespace Ui { class MixerBar; }
struct Mixer;

class MixerBar : public QWidget
{
    Q_OBJECT

public:
    explicit MixerBar (Mixer * parent, int row);
    ~MixerBar ();
		Ui::MixerBar * ui () { return m_ui; }
public slots:
		void onClickedOnOn ();
		void onClickedOnOff ();
private:
    Ui::MixerBar * m_ui;
		Mixer * m_mixer;
		int m_row;
};

