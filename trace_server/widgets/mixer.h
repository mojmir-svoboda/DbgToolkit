#pragma once
#include <QWidget>
#include <array>
#include <mixerconfig.h>
#include "appdata.h"
#include <QPixmap>

namespace Ui { class Mixer; }
struct RubberBand;
struct MixerButton;
struct VerticalLabel;

struct Mixer : QWidget
{
    Ui::Mixer * ui;
		RubberBand * m_rubberBand;
		QToolButton * m_button;
		unsigned m_rows;
		unsigned m_cols;
		QPoint m_origin;
		std::array<std::array<MixerButton *, sizeof(context_t) * CHAR_BIT>, sizeof(level_t) * CHAR_BIT> m_buttons;
		std::array<VerticalLabel *, sizeof(context_t) * CHAR_BIT> m_col_labels;
		std::array<QLabel *, sizeof(level_t) * CHAR_BIT> m_row_labels;
		MixerConfig m_config;
		QPixmap m_pixmap;
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
		void hideUnknownLabels ();

Q_OBJECT
public slots:
		void onAllButton ();
		void onNoneButton ();
		void onApplyButton ();
		void onDictionaryArrived (int type, Dict const & dict);
signals:
		void mixerChanged ();
};

