#pragma once
#include <QWidget>

namespace Ui { class MixerBarV; }
struct Mixer;

class MixerBarV : public QWidget
{
    Q_OBJECT
public:
    explicit MixerBarV (Mixer * parent, int col);
    ~MixerBarV ();

		Ui::MixerBarV * ui () { return m_ui; }

public slots:
		void onClickedOnOn ();
		void onClickedOnOff ();

private:
    Ui::MixerBarV * m_ui;
		Mixer * m_mixer;
		int m_col;
};

