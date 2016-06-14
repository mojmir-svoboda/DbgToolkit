#pragma once
#include <QWidget>
#include <array>
#include <mixerconfig.h>
#include "appdata.h"
#include <QPixmap>
#include <QTimer>

namespace Ui { class Mixer; }
struct RubberBand;
struct MixerButton;
struct VerticalLabel;

struct Mixer : QWidget
{
    Ui::Mixer * ui;
		RubberBand * m_rubberBand;
		QToolButton * m_button;
		QPoint m_origin;
		constexpr unsigned static const n_levels = sizeof(level_t) * CHAR_BIT;
		constexpr unsigned static const n_contexts = sizeof(context_t) * CHAR_BIT;
		std::array<std::array<MixerButton *, n_contexts>, n_levels> m_buttons;
		std::array<VerticalLabel *, n_contexts> m_col_labels;
		std::array<QLabel *, n_levels> m_row_labels;
		MixerConfig m_config;
		QPixmap m_pixmap;
		QTimer m_timer;
		bool m_has_dict_x;
		bool m_has_dict_y;

    explicit Mixer (QWidget * parent, QToolButton * butt, unsigned rows, unsigned cols);
    ~Mixer ();

		void startRubberBand (QPoint const & origin);
		void enlargeRubberBand (QPoint const & pt);
		void stopRubberBand (QPoint const & pt);

		void setupMixer (MixerConfig & config);
		void applyConfig (MixerConfig & config);
		void mousePressEvent (QMouseEvent * event);
		void mouseMoveEvent (QMouseEvent * event);
		void mouseReleaseEvent (QMouseEvent * event);
		void setMixerValues (mixervalues_t const & values);
		void setRowTo (int row, bool on);
		void setColTo (int col, bool on);
		void setValueTo (int row, int col, bool val);
		void addYDictionary (Dict const & d);
		void addXDictionary (Dict const & d);
		void fillMixerGaps ();
		void hideUnknownLabels ();

		template<typename T0, typename T1>
		void dataInput (T0 row, T1 col)
		{
			for (T0 b = 0; row; ++b, row >>= 1)
				if (row & 1)
				{
					for (T0 c = 0; col; ++c, col >>= 1)
						if (col & 1)
							dataInputRowCol(b, c);
				}
		}

		void addCol (unsigned c);
		void addRow (unsigned r);
		void dataInputRowCol (unsigned row, unsigned col);

		void addLeftLabel (unsigned r);
		void addTopLabel (unsigned c);
		void addButtons (unsigned r, unsigned c);

Q_OBJECT
public slots:
		void onAllButton ();
		void onNoneButton ();
		void onApplyButton ();
		void onDictionaryArrived (int type, Dict const & dict);
		void onTimerHit ();
signals:
		void mixerChanged ();
};

