#include "mixer.h"
#include "ui_mixer.h"
#include "mixerbar.h"
#include "ui_mixerbar.h"
#include "ui_mixerbarv.h"
#include "mixerbutton.h"
#include "mixerbarv.h"
#include <QToolButton>
#include <QMouseEvent>
#include "rubberband.h"
#include "label.h"

Mixer::Mixer (QWidget * parent, QToolButton * butt, unsigned rows, unsigned cols)
	: QWidget(parent)
	, ui(new Ui::Mixer)
// 	, m_rows(rows)
// 	, m_cols(cols)
	, m_rubberBand(new RubberBand(QRubberBand::Rectangle, this))
	, m_button(butt)
	, m_pixmap(cols, rows)
	, m_has_dict_x(false)
	, m_has_dict_y(false)
{
  ui->setupUi(this);
	connect(ui->allButton, SIGNAL(clicked()), this, SLOT(onAllButton()));
	connect(ui->noneButton, SIGNAL(clicked()), this, SLOT(onNoneButton()));
	connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
	m_col_labels.fill(nullptr);
	m_row_labels.fill(nullptr);
	setWindowFlags(Qt::Tool);

	m_pixmap.fill(Qt::black);
	m_button->setIcon(QIcon(m_pixmap));
}

Mixer::~Mixer ()
{
	delete ui;
	if (m_rubberBand)
		delete m_rubberBand;
}


void Mixer::addTopLabel (unsigned c)
{
	if (m_config.m_cols[c] == -1)
		return;

	m_config.m_cols[c] = c;

	QGridLayout * grid = ui->grid;

	if (QLayoutItem * li = grid->itemAtPosition(0, c + 2))
	{
	}
	else
	{
		VerticalLabel * top_label = new VerticalLabel(this, 7);
		m_col_labels[c] = top_label;
		//top_label->setText(tr("%1").arg(c));
		top_label->setAlignment(Qt::AlignLeft);
		top_label->setWordWrap(false);
		top_label->setMinimumSize(QSize(17, 64));
		top_label->setMaximumSize(QSize(17, 96));
		grid->addWidget(top_label, 0, c + 2);
	}

	if (QLayoutItem * li = grid->itemAtPosition(1, c + 2))
	{
		// flash ico
	}
	else
	{
		MixerBarV * top_bar = new MixerBarV(this, this, c);
		grid->addWidget(top_bar, 1, c + 2);
		connect(top_bar->ui()->offButton, SIGNAL(clicked()), top_bar, SLOT(onClickedOnOff()));
		connect(top_bar->ui()->onButton, SIGNAL(clicked()), top_bar, SLOT(onClickedOnOn()));
	}
}

void Mixer::addLeftLabel (unsigned r)
{
	if (m_config.m_rows[r] == -1)
		return;

	QGridLayout * grid = ui->grid;
	if (QLayoutItem * li = grid->itemAtPosition(r + 2, 0))
	{
	}
	else
	{
		m_config.m_rows[r] = r;
		QLabel * left_label = new QLabel(this);
		m_row_labels[r] = left_label;
		//left_label->setText(tr("%1").arg(r));
		left_label->setAlignment(Qt::AlignRight);
		grid->addWidget(left_label, r + 2, 0);
	}

	if (QLayoutItem * li = grid->itemAtPosition(r + 2, 1))
	{
		// flash ico
	}
	else
	{
		MixerBar * left_bar = new MixerBar(this, this, r);
		grid->addWidget(left_bar, r + 2, 1);
		connect(left_bar->ui()->offButton, SIGNAL(clicked()), left_bar, SLOT(onClickedOnOff()));
		connect(left_bar->ui()->onButton, SIGNAL(clicked()), left_bar, SLOT(onClickedOnOn()));
	}
}

void Mixer::addButtons (unsigned r, unsigned c)
{
	QGridLayout * grid = ui->grid;

	if (m_config.m_cols[c] == -1)
		return;

	if (QLayoutItem * li = grid->itemAtPosition(r + 2, c + 2))
	{
	}
	else
	{
		MixerButton * w = new MixerButton(this, r, c);
		//QToolButton:pressed{ background-color:rgb(0, 88, 64)); }
		w->setStyleSheet("QToolButton:checked{background-color:rgb(0, 88, 64); color:rgb(0, 255, 127);}");
		m_buttons[r][c] = w;
		connect(w, SIGNAL(clicked(bool)), w, SLOT(onClicked(bool)));
		grid->addWidget(w, r + 2, c + 2);
	}
}

void Mixer::setupMixer (MixerConfig & config)
{
	m_config = config;

	for (auto & outer : m_buttons)
		for (MixerButton * & b : outer)
			b = nullptr;

	for (unsigned c = 0; c < m_config.m_cols.size(); ++c)
	{
		if (m_config.m_cols[c] < 0)
			continue;
		addTopLabel(c);
	}

	for (unsigned r = 0; r < m_config.m_rows.size(); ++r)
	{
		if (m_config.m_rows[r] < 0)
			continue;
		addLeftLabel(r);

		for (unsigned c = 0; c < m_config.m_cols.size(); ++c)
		{
			if (m_config.m_cols[c] < 0)
				continue;
			addButtons(r, c);
		}
	}
}

void Mixer::addYDictionary (Dict const & d)
{
	for (level_t l = 0; l < m_row_labels.size(); ++l)
	{
		level_t const val = 1 << l;
		QString const & s = d.findNameFor(val);
		if (!s.isEmpty())
		{
			if (!m_row_labels[l])
				addLeftLabel(l);
			m_row_labels[l]->setText(s);
		}
	}
	m_has_dict_y = true;
	fillMixerGaps();
}
void Mixer::addXDictionary (Dict const & d)
{
	for (context_t c = 0; c < m_col_labels.size(); ++c)
	{
		context_t const one = 1;
		context_t const val = one << c;
		QString const & s = d.findNameFor(val);
		if (!s.isEmpty())
		{
			if (!m_col_labels[c])
				addTopLabel(c);
			m_col_labels[c]->setText(s);
		}
	}
	m_has_dict_x = true;
	fillMixerGaps();
}

void Mixer::startRubberBand (QPoint const & origin)
{
	m_origin = origin;
	m_rubberBand->setGeometry(QRect(m_origin, QSize()));
	m_rubberBand->show();
}
void Mixer::enlargeRubberBand (QPoint const & pt)
{
	m_rubberBand->setGeometry(QRect(m_origin, pt).normalized());
}

void Mixer::stopRubberBand (QPoint const & pt)
{
	m_rubberBand->hide();
	QRect box(m_origin, pt);
	// determine selection, for example using QRect::intersects()
	// and QRect::contains().
}

void Mixer::mousePressEvent (QMouseEvent * event)
{
	startRubberBand(event->pos());
}
void Mixer::mouseMoveEvent (QMouseEvent * event)
{
	enlargeRubberBand(event->pos());
}
void Mixer::mouseReleaseEvent (QMouseEvent * event)
{
	stopRubberBand(event->pos());
}

void Mixer::setValueTo (int row, int col, bool val)
{
	level_t const r = row;
	level_t const rval = 1 << r;
	if (val)
		m_config.m_state[col] |= rval;
	else
		m_config.m_state[col] &= ~rval;
}

void Mixer::setMixerValues (mixervalues_t const & values)
{
}

void Mixer::applyConfig (MixerConfig & config)
{
	m_config = config;
	QPainter painter(&m_pixmap);
	m_pixmap.fill(Qt::black);

	for (unsigned r = 0; r < m_config.m_rows.size(); ++r)
	{
		level_t const rval = 1u << r;
		for (unsigned c = 0; c < m_config.m_cols.size(); ++c)
		{
			if (MixerButton * const b = m_buttons[r][c])
			{
				level_t const l = m_config.m_state[c];
				if (l & rval)
				{
					b->setChecked(true);
					painter.setBrush(QBrush(Qt::green));
					painter.drawPoint(QPoint(c, r));
				}
				else
				{
					b->setChecked(false);
				}
			}
		}
	}
	m_button->setIcon(QIcon(m_pixmap));
}

void Mixer::setRowTo (int row, bool on)
{
	for (unsigned c = 0; c < m_config.m_cols.size(); ++c)
	{
		setValueTo(row, c, on);
		if (MixerButton * const b = m_buttons[row][c])
			b->setChecked(on);
	}
}

void Mixer::setColTo (int col, bool on)
{
	for (unsigned r = 0; r < m_config.m_rows.size(); ++r)
	{
		setValueTo(r, col, on);
		if (MixerButton * const b = m_buttons[r][col])
			b->setChecked(on);
	}
}

void Mixer::onAllButton ()
{
	for (unsigned r = 0; r < m_config.m_rows.size(); ++r)
		for (unsigned c = 0; c < m_config.m_cols.size(); ++c)
		{
			setValueTo(r, c, true);
			if (MixerButton * const b = m_buttons[r][c])
				b->setChecked(true);
		}
}
void Mixer::onNoneButton ()
{
	for (unsigned r = 0; r < m_config.m_rows.size(); ++r)
		for (unsigned c = 0; c < m_config.m_cols.size(); ++c)
		{
			setValueTo(r, c, false);
			if (MixerButton * const b = m_buttons[r][c])
				b->setChecked(false);
		}
}

void Mixer::onApplyButton ()
{
	emit mixerChanged();
}

void Mixer::onDictionaryArrived (int type, Dict const & d)
{
	switch (type)
	{
		case 0: addYDictionary(d); break;
		case 1: addXDictionary(d); break;
	}
}

void Mixer::dataInputRowCol (unsigned row, unsigned col)
{
	addTopLabel(col);
	addLeftLabel(row);
	addButtons(row, col);

	//QPainter painter(&m_pixmap);
	//m_pixmap.fill(Qt::black);

  QPainter p(&m_pixmap);
  p.setPen(Qt::blue);
  p.drawPoint(row, col);
  p.end();

	m_button->setIcon(QIcon(m_pixmap));

}

void Mixer::fillMixerGaps ()
{
	if (m_has_dict_x && m_has_dict_y)
	{
		for (unsigned r = 0; r < m_row_labels.size(); ++r)
		{
			if (m_row_labels[r] && !m_row_labels[r]->text().isEmpty())
			{
				for (unsigned c = 0; c < m_col_labels.size(); ++c)
				{
					if (m_col_labels[c] && !m_col_labels[c]->text().isEmpty())
					{
						addButtons(r, c);
					}
				}
			}
		}
	}
}

void Mixer::hideUnknownLabels ()
{
// 	if (m_has_dict_x && m_has_dict_y)
// 	{
// 		QGridLayout * grid = ui->grid;
// 
// 		for (unsigned r = 0; r < m_rows; ++r)
// 		{
// 			if (m_row_labels[r] && m_row_labels[r]->text().isEmpty())
// 			{
// 				for (unsigned c = 0; c < m_cols; ++c)
// 				{
// 					//if (m_col_labels[c]->text().isEmpty())
// 					{
// 						//grid->addWidget(nullptr, 0, c + 2); // top_label
// 						{
// 							// remove mixer button
// 							QLayoutItem * li = grid->itemAtPosition(r + 2, c + 2);
// 							grid->removeItem(li);
// 							li->widget()->hide();
// 						}
// 					}
// 				}
// 				// remove left label
// 				QLayoutItem * lill = grid->itemAtPosition(r + 2, 0);
// 				grid->removeItem(lill);
// 				lill->widget()->hide();
// 					
// 				// remove left bar
// 				QLayoutItem * lilb = grid->itemAtPosition(r + 2, 1);
// 				grid->removeItem(lilb);
// 				lilb->widget()->hide();
// 				// remove right bar
// 				QLayoutItem * lirb = grid->itemAtPosition(r + 2, m_cols + 2);
// 				grid->removeItem(lirb);
// 				lirb->widget()->hide();
// 			}
// 			else
// 			{
// 				for (unsigned c = 0; c < m_cols; ++c)
// 				{
// 					if (m_col_labels[c] && m_col_labels[c]->text().isEmpty())
// 					{
// 						// remove mixer button
// 						QLayoutItem * li = grid->itemAtPosition(r + 2, c + 2);
// 						grid->removeItem(li);
// 						li->widget()->hide();
// 					}
// 				}
// 			}
// 		}
// 		for (unsigned c = 0; c < m_cols; ++c)
// 		{
// 			if (m_col_labels[c] && m_col_labels[c]->text().isEmpty())
// 			{
// 				// remove top label
// 				QLayoutItem * litl = grid->itemAtPosition(0, c + 2);
// 				grid->removeItem(litl);
// 				litl->widget()->hide();
// 				// remove top bar
// 				QLayoutItem * litb = grid->itemAtPosition(1, c + 2);
// 				grid->removeItem(litb);
// 				litb->widget()->hide();
// 			}
// 		}
// 	}
}


