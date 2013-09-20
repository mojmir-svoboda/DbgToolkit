#include "filterbase.h"
#include <QApplication>
#include <QStyle>

FilterBase::FilterBase (QWidget * parent)
	: QWidget(parent)
	, m_enabled(true)
	, m_widget(0)
	, m_button(new QToolButton)
{
	m_button->setCheckable(true);
	m_button->setToolTip(QApplication::translate("FilterBar", "enables/disables filter", 0));
	m_button->setChecked(m_enabled);
	m_button->setIcon(grabIcon(m_enabled));
	m_button->setMinimumSize(QSize(16, 16));
	m_button->setMaximumSize(QSize(24, 16));
	connect(m_button, SIGNAL(clicked()), this, SLOT(onTabButton()));
}

FilterBase::~FilterBase ()
{
	disconnect(m_button, SIGNAL(clicked()), this, SLOT(onTabButton()));
	delete m_button;
	m_button = 0;
}

QIcon grabIcon (bool enabled)
{
	QStyle const * const style = QApplication::style();
	QIcon icon(style->standardIcon(enabled ? QStyle::SP_DialogYesButton : QStyle::SP_DialogNoButton));
	return icon;
}


void FilterBase::onTabButton ()
{
	bool const enabled = m_button->isChecked();
	bool const change = m_enabled ^ enabled;
	m_enabled = enabled;
	m_button->setIcon(grabIcon(enabled));

	if (change)
		emit filterEnabledChanged();
}

void FilterBase::applyConfig ()
{
	m_button->setChecked(m_enabled);
	m_button->setIcon(grabIcon(m_enabled));
}
