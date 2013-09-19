#include "filterbase.h"
#include <QApplication>
#include <QStyle>

QIcon grabIcon (bool enabled)
{
	QStyle const * const style = QApplication::style();
	QIcon icon(style->standardIcon(enabled ? QStyle::SP_DialogYesButton : QStyle::SP_DialogNoButton));
	return icon;
}


void FilterBase::onTabButton ()
{
	bool const enabled = m_button->isChecked();
	m_enabled = enabled;
	m_button->setIcon(grabIcon(enabled));
}

void FilterBase::applyConfig ()
{
	m_button->setChecked(m_enabled);
	m_button->setIcon(grabIcon(m_enabled));
}
